//
// Copyright 2019 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <grpc/support/port_platform.h>

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/functional/function_ref.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"

#include <grpc/impl/connectivity_state.h>
#include <grpc/support/json.h>
#include <grpc/support/log.h>

#include "src/core/ext/filters/client_channel/lb_policy/address_filtering.h"
#include "src/core/ext/filters/client_channel/lb_policy/outlier_detection/outlier_detection.h"
#include "src/core/ext/filters/client_channel/lb_policy/xds/xds_channel_args.h"
#include "src/core/ext/filters/client_channel/resolver/xds/xds_config.h"
#include "src/core/ext/xds/xds_client_stats.h"
#include "src/core/ext/xds/xds_cluster.h"
#include "src/core/ext/xds/xds_endpoint.h"
#include "src/core/ext/xds/xds_health_status.h"
#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/match.h"
#include "src/core/lib/gprpp/no_destruct.h"
#include "src/core/lib/gprpp/orphanable.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/gprpp/ref_counted_string.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/iomgr/pollset_set.h"
#include "src/core/lib/iomgr/resolved_address.h"
#include "src/core/lib/json/json.h"
#include "src/core/lib/json/json_args.h"
#include "src/core/lib/json/json_object_loader.h"
#include "src/core/lib/json/json_writer.h"
#include "src/core/lib/load_balancing/delegating_helper.h"
#include "src/core/lib/load_balancing/lb_policy.h"
#include "src/core/lib/load_balancing/lb_policy_factory.h"
#include "src/core/lib/load_balancing/lb_policy_registry.h"
#include "src/core/lib/resolver/endpoint_addresses.h"

namespace grpc_core {

TraceFlag grpc_cds_lb_trace(false, "cds_lb");

namespace {

using XdsConfig = XdsDependencyManager::XdsConfig;

constexpr absl::string_view kCds = "cds_experimental";

// Config for this LB policy.
class CdsLbConfig : public LoadBalancingPolicy::Config {
 public:
  CdsLbConfig() = default;

  CdsLbConfig(const CdsLbConfig&) = delete;
  CdsLbConfig& operator=(const CdsLbConfig&) = delete;

  CdsLbConfig(CdsLbConfig&& other) = delete;
  CdsLbConfig& operator=(CdsLbConfig&& other) = delete;

  const std::string& cluster() const { return cluster_; }
  absl::string_view name() const override { return kCds; }

  static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
    static const auto* loader = JsonObjectLoader<CdsLbConfig>()
                                    .Field("cluster", &CdsLbConfig::cluster_)
                                    .Finish();
    return loader;
  }

 private:
  std::string cluster_;
};

// CDS LB policy.
class CdsLb : public LoadBalancingPolicy {
 public:
  explicit CdsLb(Args args);

  absl::string_view name() const override { return kCds; }

  absl::Status UpdateLocked(UpdateArgs args) override;
  void ResetBackoffLocked() override;
  void ExitIdleLocked() override;

 private:
  // Delegating helper to be passed to child policy.
  using Helper = ParentOwningDelegatingChannelControlHelper<CdsLb>;

  struct LeafClusterState {
    std::string cluster_name;
    std::shared_ptr<const XdsClusterResource> cluster;
    std::shared_ptr<const XdsEndpointResource> endpoints;
    std::string resolution_note;
    bool is_logical_dns;

    // State used to retain child policy names for the priority policy.
    std::vector<size_t /*child_number*/> priority_child_numbers;
    size_t next_available_child_number = 0;

    LeafClusterState(std::string cluster_name,
                     std::shared_ptr<const XdsClusterResource> cluster,
                     std::shared_ptr<const XdsEndpointResource> endpoints,
                     std::string resolution_note, bool is_logical_dns)
        : cluster_name(std::move(cluster_name)),
          cluster(std::move(cluster)),
          endpoints(std::move(endpoints)),
          resolution_note(std::move(resolution_note)),
          is_logical_dns(is_logical_dns) {}

    std::string GetChildPolicyName(size_t priority) const;

    bool operator==(const LeafClusterState& other) const {
      return cluster_name == other.cluster_name && cluster == other.cluster &&
             endpoints == other.endpoints &&
             resolution_note == other.resolution_note &&
             is_logical_dns == other.is_logical_dns &&
             priority_child_numbers == other.priority_child_numbers &&
             next_available_child_number == other.next_available_child_number;
    }
  };
  using LeafClusterList = std::vector<LeafClusterState>;

  ~CdsLb() override;

  void ShutdownLocked() override;

  // Generates a leaf cluster list from xds_config with root cluster
  // cluster_name.  Does not populate the priority_child_numbers or
  // next_available_child_number field in each entry.  Sets
  // *lb_policy_config to point to the LB policy config from the root cluster.
  // Calls itself recursively for aggregate clusters.
  absl::Status GenerateLeafClusterList(
      const std::string& cluster_name, const XdsConfig& xds_config, int depth,
      LeafClusterList* leaf_cluster_list, const Json::Array** lb_policy_config,
      std::set<absl::string_view>* clusters_seen);

  // Populates priority_child_numbers and next_available_child_number in
  // new_list, reusing child numbers from old_list in an intelligent way to
  // avoid unnecessary churn.
  void ComputeChildNumbers(const LeafClusterList& old_list,
                           LeafClusterList* new_list);

  Json CreateChildPolicyConfig(const Json::Array& lb_policy_config);
  std::shared_ptr<EndpointAddressesIterator> CreateChildPolicyAddresses();
  std::string CreateChildPolicyResolutionNote();

  void MaybeDestroyChildPolicyLocked();

  LeafClusterList leaf_cluster_list_;

  // Child LB policy.
  OrphanablePtr<LoadBalancingPolicy> child_policy_;

  // Internal state.
  bool shutting_down_ = false;
};

//
// CdsLb::LeafClusterState
//

std::string MakeChildPolicyName(absl::string_view cluster_name,
                                size_t child_number) {
  return absl::StrCat("{cluster=", cluster_name,
                      ", child_number=", child_number, "}");
}

std::string CdsLb::LeafClusterState::GetChildPolicyName(size_t priority) const {
  return MakeChildPolicyName(cluster_name, priority_child_numbers[priority]);
}

//
// CdsLb
//

CdsLb::CdsLb(Args args) : LoadBalancingPolicy(std::move(args)) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
    gpr_log(GPR_INFO, "[cdslb %p] created", this);
  }
}

CdsLb::~CdsLb() {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
    gpr_log(GPR_INFO, "[cdslb %p] destroying cds LB policy", this);
  }
}

void CdsLb::ShutdownLocked() {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
    gpr_log(GPR_INFO, "[cdslb %p] shutting down", this);
  }
  shutting_down_ = true;
  MaybeDestroyChildPolicyLocked();
}

void CdsLb::ResetBackoffLocked() {
  if (child_policy_ != nullptr) child_policy_->ResetBackoffLocked();
}

void CdsLb::ExitIdleLocked() {
  if (child_policy_ != nullptr) child_policy_->ExitIdleLocked();
}

// We need at least one priority for each discovery mechanism, just so that we
// have a child in which to create the xds_cluster_impl policy.  This ensures
// that we properly handle the case of a discovery mechanism dropping 100% of
// calls, the OnError() case, and the OnResourceDoesNotExist() case.
const XdsEndpointResource::PriorityList& GetUpdatePriorityList(
    const XdsEndpointResource* update) {
  static const NoDestruct<XdsEndpointResource::PriorityList>
      kPriorityListWithEmptyPriority(1);
  if (update == nullptr || update->priorities.empty()) {
    return *kPriorityListWithEmptyPriority;
  }
  return update->priorities;
}

absl::Status CdsLb::UpdateLocked(UpdateArgs args) {
  // Get config.
  RefCountedPtr<CdsLbConfig> config = std::move(args.config);
  if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
    gpr_log(GPR_INFO, "[cdslb %p] received update: cluster=%s", this,
            config->cluster().c_str());
  }
  // Get xDS config.
  auto xds_config = args.args.GetObject<XdsDependencyManager::XdsConfig>();
  if (xds_config == nullptr) {
    if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
      gpr_log(GPR_INFO, "[cdslb %p] update does not include XdsConfig", this);
    }
    absl::Status status =
        absl::InternalError("xDS config not passed to CDS LB policy");
    channel_control_helper()->UpdateState(
        GRPC_CHANNEL_TRANSIENT_FAILURE, status,
        MakeRefCounted<TransientFailurePicker>(status));
    return absl::OkStatus();
  }
  // Generate cluster list.
  const Json::Array* lb_policy_config = nullptr;
  LeafClusterList leaf_cluster_list;
  std::set<absl::string_view> clusters_seen;
  absl::Status status = GenerateLeafClusterList(
      config->cluster(), *xds_config, /*depth=*/0, &leaf_cluster_list,
      &lb_policy_config, &clusters_seen);
  if (!status.ok()) {
    // If we don't already have a child policy, then report TF.
    // Otherwise, just ignore the update and stick with our previous config.
    if (child_policy_ == nullptr) {
      status = absl::Status(status.code(), absl::StrCat(config->cluster(), ": ",
                                                        status.message()));
      channel_control_helper()->UpdateState(
          GRPC_CHANNEL_TRANSIENT_FAILURE, status,
          MakeRefCounted<TransientFailurePicker>(status));
    }
    return absl::OkStatus();
  }
  GPR_ASSERT(lb_policy_config != nullptr);
  // If there is no change, do nothing.
  if (leaf_cluster_list_ == leaf_cluster_list) return absl::OkStatus();
  // We have a new leaf cluster list.  Compute the priority child numbers
  // for each entry.
  ComputeChildNumbers(leaf_cluster_list_, &leaf_cluster_list);
  // Swap new list into place.
  leaf_cluster_list_ = std::move(leaf_cluster_list);
  // Construct child policy config.
  Json json = CreateChildPolicyConfig(*lb_policy_config);
  auto child_config =
      CoreConfiguration::Get().lb_policy_registry().ParseLoadBalancingConfig(
          json);
  if (!child_config.ok()) {
    // This should never happen, but if it does, we basically have no
    // way to fix it, so we put the channel in TRANSIENT_FAILURE.
    gpr_log(GPR_ERROR,
            "[cdslb %p] error parsing generated child policy config -- "
            "will put channel in TRANSIENT_FAILURE: %s",
            this, child_config.status().ToString().c_str());
    absl::Status status = absl::InternalError(
        absl::StrCat(config->cluster(), ": error parsing child policy config: ",
                     child_config.status().message()));
    channel_control_helper()->UpdateState(
        GRPC_CHANNEL_TRANSIENT_FAILURE, status,
        MakeRefCounted<TransientFailurePicker>(status));
    return absl::OkStatus();
  }
  // Create child policy if not already present.
  if (child_policy_ == nullptr) {
    LoadBalancingPolicy::Args lb_args;
    lb_args.work_serializer = work_serializer();
    lb_args.args = args.args;
    lb_args.channel_control_helper = std::make_unique<Helper>(Ref());
    child_policy_ =
        CoreConfiguration::Get().lb_policy_registry().CreateLoadBalancingPolicy(
            (*child_config)->name(), std::move(lb_args));
    if (child_policy_ == nullptr) {
      absl::Status status = absl::UnavailableError(
          absl::StrCat(config->cluster(), ": failed to create child policy"));
      channel_control_helper()->UpdateState(
          GRPC_CHANNEL_TRANSIENT_FAILURE, status,
          MakeRefCounted<TransientFailurePicker>(status));
      return absl::OkStatus();
    }
    grpc_pollset_set_add_pollset_set(child_policy_->interested_parties(),
                                     interested_parties());
    if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
      gpr_log(GPR_INFO, "[cdslb %p] created child policy %s (%p)", this,
              std::string((*child_config)->name()).c_str(),
              child_policy_.get());
    }
  }
  // Update child policy.
  UpdateArgs update_args;
  update_args.config = std::move(*child_config);
  update_args.addresses = CreateChildPolicyAddresses();
  update_args.resolution_note = CreateChildPolicyResolutionNote();
  update_args.args = args.args;
  return child_policy_->UpdateLocked(std::move(update_args));
}

absl::Status CdsLb::GenerateLeafClusterList(
    const std::string& cluster_name, const XdsConfig& xds_config, int depth,
    LeafClusterList* leaf_cluster_list, const Json::Array** lb_policy_config,
    std::set<absl::string_view>* clusters_seen) {
  if (depth == kMaxXdsAggregateClusterRecursionDepth) {
    return absl::InternalError("aggregate cluster graph exceeds max depth");
  }
  // Ignore if this cluster was already added from some other branch.
  if (!clusters_seen->insert(cluster_name).second) return absl::OkStatus();
  // Find cluster resource.
  auto it = xds_config.clusters.find(cluster_name);
  if (it == xds_config.clusters.end()) {
    return absl::InternalError(
        absl::StrCat("cluster ", cluster_name, " not found in xDS config"));
  }
  if (!it->second.ok()) return it->second.status();
  auto& cluster_resource = *it->second;
  if (cluster_resource == nullptr) {
    return absl::InternalError(
        absl::StrCat("cluster ", cluster_name, " null in xDS config"));
  }
  // Save xDS LB config from the root cluster.
  if (depth == 0) *lb_policy_config = &cluster_resource->lb_policy_config;
  // Find endpoints.
  return Match(
      cluster_resource->type,
      // EDS
      [&](const XdsClusterResource::Eds& eds) {
        auto it = xds_config.endpoints.find(eds.eds_service_name);
        if (it == xds_config.endpoints.end()) {
          return absl::InternalError(
              absl::StrCat("cluster ", cluster_name, ": EDS resource ",
                           eds.eds_service_name, " not found"));
        }
        leaf_cluster_list->emplace_back(cluster_name, cluster_resource,
                                        it->second.endpoints,
                                        it->second.resolution_note,
                                        /*is_logical_dns=*/false);
        return absl::OkStatus();
      },
      // LOGICAL_DNS
      [&](const XdsClusterResource::LogicalDns& logical_dns) {
        auto it = xds_config.dns_results.find(logical_dns.hostname);
        if (it == xds_config.dns_results.end()) {
          return absl::InternalError(
              absl::StrCat("cluster ", cluster_name, ": EDS resource ",
                           logical_dns.hostname, " not found"));
        }
        leaf_cluster_list->emplace_back(cluster_name, cluster_resource,
                                        it->second.endpoints,
                                        it->second.resolution_note,
                                        /*is_logical_dns=*/true);
        return absl::OkStatus();
      },
      // Aggregate
      [&](const XdsClusterResource::Aggregate& aggregate) {
        for (const std::string& child_name :
             aggregate.prioritized_cluster_names) {
          absl::Status status = GenerateLeafClusterList(
              child_name, xds_config, depth + 1, leaf_cluster_list, nullptr,
              clusters_seen);
          if (!status.ok()) return status;
        }
        return absl::OkStatus();
      });
}

void CdsLb::ComputeChildNumbers(const LeafClusterList& old_list,
                                LeafClusterList* new_list) {
  // First, build some maps from locality to child number and the reverse
  // from old_list.
  struct ClusterChildNameState {
    std::map<XdsLocalityName*, size_t /*child_number*/, XdsLocalityName::Less>
        locality_child_map;
    std::map<size_t, std::set<XdsLocalityName*, XdsLocalityName::Less>>
        child_locality_map;
    size_t next_available_child_number;
  };
  std::map<absl::string_view, ClusterChildNameState> cluster_child_name_map;
  for (const LeafClusterState& state : old_list) {
    const auto& prev_priority_list =
        GetUpdatePriorityList(state.endpoints.get());
    auto& child_name_state = cluster_child_name_map[state.cluster_name];
    child_name_state.next_available_child_number =
        state.next_available_child_number;
    for (size_t priority = 0; priority < prev_priority_list.size();
         ++priority) {
      size_t child_number = state.priority_child_numbers[priority];
      const auto& localities = prev_priority_list[priority].localities;
      for (const auto& p : localities) {
        XdsLocalityName* locality_name = p.first;
        child_name_state.locality_child_map[locality_name] = child_number;
        child_name_state.child_locality_map[child_number].insert(locality_name);
      }
    }
  }
  // Now populate priority child numbers in the new list based on
  // cluster_child_name_map.
  for (LeafClusterState& state : *new_list) {
    auto it = cluster_child_name_map.find(state.cluster_name);
    ClusterChildNameState tmp_state;
    auto& child_name_state =
        it == cluster_child_name_map.end() ? tmp_state : it->second;
    state.next_available_child_number =
        child_name_state.next_available_child_number;
    const XdsEndpointResource::PriorityList& priority_list =
        GetUpdatePriorityList(state.endpoints.get());
    for (size_t priority = 0; priority < priority_list.size(); ++priority) {
      const auto& localities = priority_list[priority].localities;
      absl::optional<size_t> child_number;
      // If one of the localities in this priority already existed, reuse its
      // child number.
      for (const auto& p : localities) {
        XdsLocalityName* locality_name = p.first;
        if (!child_number.has_value()) {
          auto it = child_name_state.locality_child_map.find(locality_name);
          if (it != child_name_state.locality_child_map.end()) {
            child_number = it->second;
            child_name_state.locality_child_map.erase(it);
            // Remove localities that *used* to be in this child number, so
            // that we don't incorrectly reuse this child number for a
            // subsequent priority.
            for (XdsLocalityName* old_locality :
                 child_name_state.child_locality_map[*child_number]) {
              child_name_state.locality_child_map.erase(old_locality);
            }
          }
        } else {
          // Remove all localities that are now in this child number, so
          // that we don't accidentally reuse this child number for a
          // subsequent priority.
          child_name_state.locality_child_map.erase(locality_name);
        }
      }
      // If we didn't find an existing child number, assign a new one.
      if (!child_number.has_value()) {
        for (child_number = state.next_available_child_number;
             child_name_state.child_locality_map.find(*child_number) !=
             child_name_state.child_locality_map.end();
             ++(*child_number)) {
        }
        state.next_available_child_number = *child_number + 1;
        // Add entry so we know that the child number is in use.
        // (Don't need to add the list of localities, since we won't use them.)
        child_name_state.child_locality_map[*child_number];
      }
      state.priority_child_numbers.push_back(*child_number);
    }
  }
}

class PriorityEndpointIterator : public EndpointAddressesIterator {
 public:
  struct ClusterEntry {
    std::shared_ptr<const XdsEndpointResource> endpoints;
    std::string cluster_name;
    std::vector<size_t /*child_number*/> priority_child_numbers;

    ClusterEntry(std::shared_ptr<const XdsEndpointResource> resource,
                 std::string cluster, std::vector<size_t> child_numbers)
        : endpoints(std::move(resource)),
          cluster_name(std::move(cluster)),
          priority_child_numbers(std::move(child_numbers)) {}

    std::string GetChildPolicyName(size_t priority) const {
      return MakeChildPolicyName(cluster_name,
                                 priority_child_numbers[priority]);
    }
  };

  explicit PriorityEndpointIterator(std::vector<ClusterEntry> results)
      : results_(std::move(results)) {}

  void ForEach(absl::FunctionRef<void(const EndpointAddresses&)> callback)
      const override {
    for (const auto& entry : results_) {
      const auto& priority_list = GetUpdatePriorityList(entry.endpoints.get());
      for (size_t priority = 0; priority < priority_list.size(); ++priority) {
        const auto& priority_entry = priority_list[priority];
        std::string priority_child_name = entry.GetChildPolicyName(priority);
        for (const auto& p : priority_entry.localities) {
          const auto& locality_name = p.first;
          const auto& locality = p.second;
          std::vector<RefCountedStringValue> hierarchical_path = {
              RefCountedStringValue(priority_child_name),
              RefCountedStringValue(locality_name->AsHumanReadableString())};
          auto hierarchical_path_attr =
              MakeRefCounted<HierarchicalPathArg>(std::move(hierarchical_path));
          for (const auto& endpoint : locality.endpoints) {
            uint32_t endpoint_weight =
                locality.lb_weight *
                endpoint.args().GetInt(GRPC_ARG_ADDRESS_WEIGHT).value_or(1);
            callback(EndpointAddresses(
                endpoint.addresses(),
                endpoint.args()
                    .SetObject(hierarchical_path_attr)
                    .Set(GRPC_ARG_ADDRESS_WEIGHT, endpoint_weight)
                    .SetObject(locality_name->Ref())
                    .Set(GRPC_ARG_XDS_LOCALITY_WEIGHT, locality.lb_weight)));
          }
        }
      }
    }
  }

 private:
  std::vector<ClusterEntry> results_;
};

std::shared_ptr<EndpointAddressesIterator> CdsLb::CreateChildPolicyAddresses() {
  std::vector<PriorityEndpointIterator::ClusterEntry> entries;
  entries.reserve(leaf_cluster_list_.size());
  for (const auto& state : leaf_cluster_list_) {
    entries.emplace_back(state.endpoints, state.cluster_name,
                         state.priority_child_numbers);
  }
  return std::make_shared<PriorityEndpointIterator>(std::move(entries));
}

std::string CdsLb::CreateChildPolicyResolutionNote() {
  std::vector<absl::string_view> resolution_notes;
  for (const auto& state : leaf_cluster_list_) {
    if (!state.resolution_note.empty()) {
      resolution_notes.push_back(state.resolution_note);
    }
  }
  return absl::StrJoin(resolution_notes, "; ");
}

Json CdsLb::CreateChildPolicyConfig(const Json::Array& lb_policy_config) {
  Json::Object priority_children;
  Json::Array priority_priorities;
  for (const auto& state : leaf_cluster_list_) {
    const auto& priority_list = GetUpdatePriorityList(state.endpoints.get());
    const auto& cluster_resource = *state.cluster;
    for (size_t priority = 0; priority < priority_list.size(); ++priority) {
      // Determine what xDS LB policy to use.
      Json child_policy;
      if (state.is_logical_dns) {
        child_policy = Json::FromArray({
            Json::FromObject({
                {"pick_first", Json::FromObject({})},
            }),
        });
      } else {
        child_policy = Json::FromArray(lb_policy_config);
      }
      // Wrap the xDS LB policy in the xds_override_host policy.
      Json::Object xds_override_host_lb_config = {
          {"childPolicy", std::move(child_policy)},
      };
      if (!cluster_resource.override_host_statuses.empty()) {
        Json::Array status_list;
        for (const auto& status : cluster_resource.override_host_statuses) {
          status_list.emplace_back(Json::FromString(status.ToString()));
        }
        xds_override_host_lb_config["overrideHostStatus"] =
            Json::FromArray(std::move(status_list));
      }
      Json::Array xds_override_host_config = {Json::FromObject({
          {"xds_override_host_experimental",
           Json::FromObject(std::move(xds_override_host_lb_config))},
      })};
      // Wrap it in the xds_cluster_impl policy.
      Json::Array drop_categories;
      if (state.endpoints != nullptr &&
          state.endpoints->drop_config != nullptr) {
        for (const auto& category :
             state.endpoints->drop_config->drop_category_list()) {
          drop_categories.push_back(Json::FromObject({
              {"category", Json::FromString(category.name)},
              {"requests_per_million",
               Json::FromNumber(category.parts_per_million)},
          }));
        }
      }
      Json::Object xds_cluster_impl_config = {
          {"clusterName", Json::FromString(state.cluster_name)},
          {"childPolicy", Json::FromArray(std::move(xds_override_host_config))},
          {"dropCategories", Json::FromArray(std::move(drop_categories))},
          {"maxConcurrentRequests",
           Json::FromNumber(cluster_resource.max_concurrent_requests)},
      };
      auto* eds = absl::get_if<XdsClusterResource::Eds>(&cluster_resource.type);
      if (eds != nullptr) {
        xds_cluster_impl_config["edsServiceName"] =
            Json::FromString(eds->eds_service_name);
      }
      if (cluster_resource.lrs_load_reporting_server.has_value()) {
        xds_cluster_impl_config["lrsLoadReportingServer"] =
            cluster_resource.lrs_load_reporting_server->ToJson();
      }
      // Wrap it in the outlier_detection policy.
      Json::Object outlier_detection_config;
      if (cluster_resource.outlier_detection.has_value()) {
        auto& outlier_detection_update = *cluster_resource.outlier_detection;
        outlier_detection_config["interval"] =
            Json::FromString(outlier_detection_update.interval.ToJsonString());
        outlier_detection_config["baseEjectionTime"] = Json::FromString(
            outlier_detection_update.base_ejection_time.ToJsonString());
        outlier_detection_config["maxEjectionTime"] = Json::FromString(
            outlier_detection_update.max_ejection_time.ToJsonString());
        outlier_detection_config["maxEjectionPercent"] =
            Json::FromNumber(outlier_detection_update.max_ejection_percent);
        if (outlier_detection_update.success_rate_ejection.has_value()) {
          outlier_detection_config["successRateEjection"] = Json::FromObject({
              {"stdevFactor",
               Json::FromNumber(outlier_detection_update.success_rate_ejection
                                    ->stdev_factor)},
              {"enforcementPercentage",
               Json::FromNumber(outlier_detection_update.success_rate_ejection
                                    ->enforcement_percentage)},
              {"minimumHosts",
               Json::FromNumber(outlier_detection_update.success_rate_ejection
                                    ->minimum_hosts)},
              {"requestVolume",
               Json::FromNumber(outlier_detection_update.success_rate_ejection
                                    ->request_volume)},
          });
        }
        if (outlier_detection_update.failure_percentage_ejection.has_value()) {
          outlier_detection_config["failurePercentageEjection"] =
              Json::FromObject({
                  {"threshold",
                   Json::FromNumber(
                       outlier_detection_update.failure_percentage_ejection
                           ->threshold)},
                  {"enforcementPercentage",
                   Json::FromNumber(
                       outlier_detection_update.failure_percentage_ejection
                           ->enforcement_percentage)},
                  {"minimumHosts",
                   Json::FromNumber(
                       outlier_detection_update.failure_percentage_ejection
                           ->minimum_hosts)},
                  {"requestVolume",
                   Json::FromNumber(
                       outlier_detection_update.failure_percentage_ejection
                           ->request_volume)},
              });
        }
      }
      outlier_detection_config["childPolicy"] =
          Json::FromArray({Json::FromObject({
              {"xds_cluster_impl_experimental",
               Json::FromObject(std::move(xds_cluster_impl_config))},
          })});
      Json locality_picking_policy = Json::FromArray({Json::FromObject({
          {"outlier_detection_experimental",
           Json::FromObject(std::move(outlier_detection_config))},
      })});
      // Add priority entry, with the appropriate child name.
      std::string child_name = state.GetChildPolicyName(priority);
      priority_priorities.emplace_back(Json::FromString(child_name));
      Json::Object child_config = {
          {"config", std::move(locality_picking_policy)},
      };
      if (!state.is_logical_dns) {
        child_config["ignore_reresolution_requests"] = Json::FromBool(true);
      }
      priority_children[child_name] = Json::FromObject(std::move(child_config));
    }
  }
  Json json = Json::FromArray({Json::FromObject({
      {"priority_experimental",
       Json::FromObject({
           {"children", Json::FromObject(std::move(priority_children))},
           {"priorities", Json::FromArray(std::move(priority_priorities))},
       })},
  })});
  if (GRPC_TRACE_FLAG_ENABLED(grpc_cds_lb_trace)) {
    gpr_log(GPR_INFO, "[cdslb %p] generated config for child policy: %s", this,
            JsonDump(json, /*indent=*/1).c_str());
  }
  return json;
}

void CdsLb::MaybeDestroyChildPolicyLocked() {
  if (child_policy_ != nullptr) {
    grpc_pollset_set_del_pollset_set(child_policy_->interested_parties(),
                                     interested_parties());
    child_policy_.reset();
  }
}

//
// factory
//

class CdsLbFactory : public LoadBalancingPolicyFactory {
 public:
  OrphanablePtr<LoadBalancingPolicy> CreateLoadBalancingPolicy(
      LoadBalancingPolicy::Args args) const override {
    return MakeOrphanable<CdsLb>(std::move(args));
  }

  absl::string_view name() const override { return kCds; }

  absl::StatusOr<RefCountedPtr<LoadBalancingPolicy::Config>>
  ParseLoadBalancingConfig(const Json& json) const override {
    return LoadFromJson<RefCountedPtr<CdsLbConfig>>(
        json, JsonArgs(), "errors validating cds LB policy config");
  }
};

}  // namespace

void RegisterCdsLbPolicy(CoreConfiguration::Builder* builder) {
  builder->lb_policy_registry()->RegisterLoadBalancingPolicyFactory(
      std::make_unique<CdsLbFactory>());
}

}  // namespace grpc_core

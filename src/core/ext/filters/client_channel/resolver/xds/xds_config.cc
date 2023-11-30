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

#include "src/core/ext/filters/client_channel/resolver/xds/xds_config.h"

#include "absl/strings/str_join.h"

#include "src/core/ext/filters/client_channel/resolver/xds/xds_resolver_trace.h"
#include "src/core/ext/xds/xds_routing.h"
#include "src/core/lib/gprpp/match.h"

namespace grpc_core {

namespace {

// Max depth of aggregate cluster dependency graph.
constexpr int kMaxXdsAggregateClusterRecursionDepth = 16;

}  // namespace

//
// XdsDependencyManager::ListenerWatcher
//

class XdsDependencyManager::ListenerWatcher
    : public XdsListenerResourceType::WatcherInterface {
 public:
  explicit ListenerWatcher(RefCountedPtr<XdsDependencyManager> config_watcher)
      : config_watcher_(std::move(config_watcher)) {}

  void OnResourceChanged(
      std::shared_ptr<const XdsListenerResource> listener) override {
    config_watcher_->work_serializer_->Run(
        [config_watcher = config_watcher_,
         listener = std::move(listener)]() mutable {
          config_watcher->OnListenerUpdate(std::move(listener));
        },
        DEBUG_LOCATION);
  }

  void OnError(absl::Status status) override {
    config_watcher_->work_serializer_->Run(
        [config_watcher = config_watcher_,
         status = std::move(status)]() mutable {
          config_watcher->OnError(config_watcher->listener_resource_name_,
                                  std::move(status));
        },
        DEBUG_LOCATION);
  }

  void OnResourceDoesNotExist() override {
    config_watcher_->work_serializer_->Run(
        [config_watcher = config_watcher_]() {
          config_watcher->OnResourceDoesNotExist(
              absl::StrCat(config_watcher->listener_resource_name_,
                           ": xDS listener resource does not exist"));
        },
        DEBUG_LOCATION);
  }

 private:
  RefCountedPtr<XdsDependencyManager> config_watcher_;
};

//
// XdsDependencyManager::RouteConfigWatcher
//

class XdsDependencyManager::RouteConfigWatcher
    : public XdsRouteConfigResourceType::WatcherInterface {
 public:
  RouteConfigWatcher(RefCountedPtr<XdsDependencyManager> config_watcher,
                     std::string name)
      : config_watcher_(std::move(config_watcher)), name_(std::move(name)) {}

  void OnResourceChanged(
      std::shared_ptr<const XdsRouteConfigResource> route_config) override {
    RefCountedPtr<RouteConfigWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self),
         route_config = std::move(route_config)]() mutable {
          self->config_watcher_->OnRouteConfigUpdate(self->name_,
                                                     std::move(route_config));
        },
        DEBUG_LOCATION);
  }

  void OnError(absl::Status status) override {
    RefCountedPtr<RouteConfigWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self), status = std::move(status)]() mutable {
          self->config_watcher_->OnError(self->name_, std::move(status));
        },
        DEBUG_LOCATION);
  }

  void OnResourceDoesNotExist() override {
    RefCountedPtr<RouteConfigWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self)]() {
          self->config_watcher_->OnResourceDoesNotExist(absl::StrCat(
              self->name_,
              ": xDS route configuration resource does not exist"));
        },
        DEBUG_LOCATION);
  }

 private:
  RefCountedPtr<XdsDependencyManager> config_watcher_;
  std::string name_;
};

//
// XdsDependencyManager::ClusterWatcher
//

class XdsDependencyManager::ClusterWatcher
    : public XdsClusterResourceType::WatcherInterface {
 public:
  ClusterWatcher(RefCountedPtr<XdsDependencyManager> config_watcher,
                 std::string name)
      : config_watcher_(std::move(config_watcher)), name_(std::move(name)) {}

  void OnResourceChanged(
      std::shared_ptr<const XdsClusterResource> cluster) override {
    RefCountedPtr<ClusterWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self), cluster = std::move(cluster)]() mutable {
          self->config_watcher_->OnClusterUpdate(self->name_,
                                                 std::move(cluster));
        },
        DEBUG_LOCATION);
  }

  void OnError(absl::Status status) override {
    RefCountedPtr<ClusterWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self), status = std::move(status)]() mutable {
          self->config_watcher_->OnClusterError(self->name_, std::move(status));
        },
        DEBUG_LOCATION);
  }

  void OnResourceDoesNotExist() override {
    RefCountedPtr<ClusterWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self)]() {
          self->config_watcher_->OnClusterDoesNotExist(self->name_);
        },
        DEBUG_LOCATION);
  }

 private:
  RefCountedPtr<XdsDependencyManager> config_watcher_;
  std::string name_;
};

//
// XdsDependencyManager::EndpointWatcher
//

class XdsDependencyManager::EndpointWatcher
    : public XdsEndpointResourceType::WatcherInterface {
 public:
  EndpointWatcher(RefCountedPtr<XdsDependencyManager> config_watcher,
                  std::string name)
      : config_watcher_(std::move(config_watcher)), name_(std::move(name)) {}

  void OnResourceChanged(
      std::shared_ptr<const XdsEndpointResource> endpoint) override {
    RefCountedPtr<EndpointWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self), endpoint = std::move(endpoint)]() mutable {
          self->config_watcher_->OnEndpointUpdate(self->name_,
                                                  std::move(endpoint));
        },
        DEBUG_LOCATION);
  }

  void OnError(absl::Status status) override {
    RefCountedPtr<EndpointWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self), status = std::move(status)]() mutable {
          self->config_watcher_->OnEndpointError(self->name_,
                                                 std::move(status));
        },
        DEBUG_LOCATION);
  }

  void OnResourceDoesNotExist() override {
    RefCountedPtr<EndpointWatcher> self = Ref();
    config_watcher_->work_serializer_->Run(
        [self = std::move(self)]() {
          self->config_watcher_->OnEndpointDoesNotExist(self->name_);
        },
        DEBUG_LOCATION);
  }

 private:
  RefCountedPtr<XdsDependencyManager> config_watcher_;
  std::string name_;
};

//
// XdsDependencyManager::ClusterSubscription
//

void XdsDependencyManager::ClusterSubscription::Orphan() {
  dependency_mgr_->work_serializer_->Run(
      [self = WeakRef()]() {
        self->dependency_mgr_->OnClusterSubscriptionUnref(self->cluster_name_,
                                                          self.get());
      },
      DEBUG_LOCATION);
}

//
// XdsDependencyManager
//

XdsDependencyManager::XdsDependencyManager(
    RefCountedPtr<GrpcXdsClient> xds_client,
    std::shared_ptr<WorkSerializer> work_serializer,
    std::unique_ptr<Watcher> watcher, std::string data_plane_authority,
    std::string listener_resource_name)
    : xds_client_(std::move(xds_client)),
      work_serializer_(std::move(work_serializer)),
      watcher_(std::move(watcher)),
      data_plane_authority_(std::move(data_plane_authority)),
      listener_resource_name_(std::move(listener_resource_name)) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO,
            "[XdsDependencyManager %p] starting watch for listener %s", this,
            listener_resource_name_.c_str());
  }
  auto listener_watcher = MakeRefCounted<ListenerWatcher>(Ref());
  listener_watcher_ = listener_watcher.get();
  XdsListenerResourceType::StartWatch(
      xds_client_.get(), listener_resource_name_, std::move(listener_watcher));
}

void XdsDependencyManager::Orphan() {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] shutting down", this);
  }
  if (listener_watcher_ != nullptr) {
    XdsListenerResourceType::CancelWatch(
        xds_client_.get(), listener_resource_name_, listener_watcher_,
        /*delay_unsubscription=*/false);
  }
  if (route_config_watcher_ != nullptr) {
    XdsRouteConfigResourceType::CancelWatch(
        xds_client_.get(), route_config_name_, route_config_watcher_,
        /*delay_unsubscription=*/false);
  }
  for (const auto& p : cluster_watchers_) {
    XdsClusterResourceType::CancelWatch(xds_client_.get(), p.first,
                                        p.second.watcher,
                                        /*delay_unsubscription=*/false);
  }
  for (const auto& p : endpoint_watchers_) {
    XdsEndpointResourceType::CancelWatch(xds_client_.get(), p.first,
                                         p.second.watcher,
                                         /*delay_unsubscription=*/false);
  }
  cluster_subscriptions_.clear();
  xds_client_.reset();
  for (auto& p : dns_resolvers_) {
    p.second.resolver.reset();
  }
  Unref();
}

void XdsDependencyManager::OnListenerUpdate(
    std::shared_ptr<const XdsListenerResource> listener) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] received Listener update",
            this);
  }
  if (xds_client_ == nullptr) return;
  const auto* hcm = absl::get_if<XdsListenerResource::HttpConnectionManager>(
      &listener->listener);
  if (hcm == nullptr) {
    return OnError(listener_resource_name_,
                   absl::UnavailableError("not an API listener"));
  }
  current_listener_ = std::move(listener);
  Match(
      hcm->route_config,
      // RDS resource name
      [&](const std::string& rds_name) {
        // If the RDS name changed, update the RDS watcher.
        // Note that this will be true on the initial update, because
        // route_config_name_ will be empty.
        if (route_config_name_ != rds_name) {
          // If we already had a watch (i.e., if the previous config had
          // a different RDS name), stop the previous watch.
          // There will be no previous watch if either (a) this is the
          // initial resource update or (b) the previous Listener had an
          // inlined RouteConfig.
          if (route_config_watcher_ != nullptr) {
            XdsRouteConfigResourceType::CancelWatch(
                xds_client_.get(), route_config_name_, route_config_watcher_,
                /*delay_unsubscription=*/true);
            route_config_watcher_ = nullptr;
          }
          // Start watch for the new RDS resource name.
          route_config_name_ = rds_name;
          if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
            gpr_log(
                GPR_INFO,
                "[XdsDependencyManager %p] starting watch for route config %s",
                this, route_config_name_.c_str());
          }
          auto watcher =
              MakeRefCounted<RouteConfigWatcher>(Ref(), route_config_name_);
          route_config_watcher_ = watcher.get();
          XdsRouteConfigResourceType::StartWatch(
              xds_client_.get(), route_config_name_, std::move(watcher));
        } else {
          // RDS resource name has not changed, so no watch needs to be
          // updated, but we still need to propagate any changes in the
          // HCM config (e.g., the list of HTTP filters).
          MaybeReportUpdate();
        }
      },
      // inlined RouteConfig
      [&](const std::shared_ptr<const XdsRouteConfigResource>& route_config) {
        // If the previous update specified an RDS resource instead of
        // having an inlined RouteConfig, we need to cancel the RDS watch.
        if (route_config_watcher_ != nullptr) {
          XdsRouteConfigResourceType::CancelWatch(
              xds_client_.get(), route_config_name_, route_config_watcher_);
          route_config_watcher_ = nullptr;
          route_config_name_.clear();
        }
        OnRouteConfigUpdate("", route_config);
      });
}

namespace {

class XdsVirtualHostListIterator : public XdsRouting::VirtualHostListIterator {
 public:
  explicit XdsVirtualHostListIterator(
      const std::vector<XdsRouteConfigResource::VirtualHost>* virtual_hosts)
      : virtual_hosts_(virtual_hosts) {}

  size_t Size() const override { return virtual_hosts_->size(); }

  const std::vector<std::string>& GetDomainsForVirtualHost(
      size_t index) const override {
    return (*virtual_hosts_)[index].domains;
  }

 private:
  const std::vector<XdsRouteConfigResource::VirtualHost>* virtual_hosts_;
};

}  // namespace

void XdsDependencyManager::OnRouteConfigUpdate(
    const std::string& name,
    std::shared_ptr<const XdsRouteConfigResource> route_config) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO,
            "[XdsDependencyManager %p] received RouteConfig update for %s",
            this, name.empty() ? "<inline>" : name.c_str());
  }
  if (xds_client_ == nullptr) return;
  // Ignore updates for stale names.
  if (name.empty()) {
    if (!route_config_name_.empty()) return;
  } else {
    if (name != route_config_name_) return;
  }
  // Find the relevant VirtualHost from the RouteConfiguration.
  // If the resource doesn't have the right vhost, fail without updating
  // our data.
  auto vhost_index = XdsRouting::FindVirtualHostForDomain(
      XdsVirtualHostListIterator(&route_config->virtual_hosts),
      data_plane_authority_);
  if (!vhost_index.has_value()) {
    OnError(route_config_name_.empty() ? listener_resource_name_
                                       : route_config_name_,
            absl::UnavailableError(
                absl::StrCat("could not find VirtualHost for ",
                             data_plane_authority_, " in RouteConfiguration")));
    return;
  }
  // Update our data.
  current_route_config_ = std::move(route_config);
  current_virtual_host_ = &current_route_config_->virtual_hosts[*vhost_index];
  clusters_from_route_config_ =
      GetClustersFromRouteConfig(*current_route_config_);
  MaybeReportUpdate();
}

void XdsDependencyManager::OnError(std::string context, absl::Status status) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO,
            "[XdsDependencyManager %p] received Listener or RouteConfig "
            "error: %s %s",
            this, context.c_str(), status.ToString().c_str());
  }
  if (xds_client_ == nullptr) return;
  if (current_virtual_host_ != nullptr) return;
  watcher_->OnError(context, std::move(status));
}

void XdsDependencyManager::OnResourceDoesNotExist(std::string context) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] %s", this, context.c_str());
  }
  if (xds_client_ == nullptr) return;
  current_virtual_host_ = nullptr;
  watcher_->OnResourceDoesNotExist(std::move(context));
}

void XdsDependencyManager::OnClusterUpdate(
    const std::string& name,
    std::shared_ptr<const XdsClusterResource> cluster) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] received Cluster update: %s",
            this, name.c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = cluster_watchers_.find(name);
  if (it == cluster_watchers_.end()) return;
  it->second.update = std::move(cluster);
  MaybeReportUpdate();
}

void XdsDependencyManager::OnClusterError(const std::string& name,
                                          absl::Status status) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] received Cluster error: %s %s",
            this, name.c_str(), status.ToString().c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = cluster_watchers_.find(name);
  if (it == cluster_watchers_.end()) return;
  if (it->second.update.value_or(nullptr) == nullptr) {
    it->second.update =
        absl::Status(status.code(), absl::StrCat(name, ": ", status.message()));
  }
  MaybeReportUpdate();
}

void XdsDependencyManager::OnClusterDoesNotExist(const std::string& name) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] Cluster does not exist: %s",
            this, name.c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = cluster_watchers_.find(name);
  if (it == cluster_watchers_.end()) return;
  it->second.update = absl::UnavailableError(
      absl::StrCat("CDS resource ", name, " does not exist"));
  MaybeReportUpdate();
}

void XdsDependencyManager::OnEndpointUpdate(
    const std::string& name,
    std::shared_ptr<const XdsEndpointResource> endpoint) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] received Endpoint update: %s",
            this, name.c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = endpoint_watchers_.find(name);
  if (it == endpoint_watchers_.end()) return;
  if (endpoint->priorities.empty()) {
    it->second.update.resolution_note =
        absl::StrCat("EDS resource ", name, " contains no localities");
  } else {
    std::set<std::string> empty_localities;
    for (const auto& priority : endpoint->priorities) {
      for (const auto& p : priority.localities) {
        if (p.second.endpoints.empty()) {
          empty_localities.insert(p.first->AsHumanReadableString());
        }
      }
    }
    if (!empty_localities.empty()) {
      it->second.update.resolution_note =
          absl::StrCat("EDS resource ", name, " contains empty localities: [",
                       absl::StrJoin(empty_localities, "; "), "]");
    }
  }
  it->second.update.endpoints = std::move(endpoint);
  MaybeReportUpdate();
}

void XdsDependencyManager::OnEndpointError(const std::string& name,
                                           absl::Status status) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO,
            "[XdsDependencyManager %p] received Endpoint error: %s %s", this,
            name.c_str(), status.ToString().c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = endpoint_watchers_.find(name);
  if (it == endpoint_watchers_.end()) return;
  if (it->second.update.endpoints == nullptr) {
    it->second.update.resolution_note =
        absl::StrCat("EDS resource ", name, ": ", status.ToString());
    MaybeReportUpdate();
  }
}

void XdsDependencyManager::OnEndpointDoesNotExist(const std::string& name) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] Endpoint does not exist: %s",
            this, name.c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = endpoint_watchers_.find(name);
  if (it == endpoint_watchers_.end()) return;
  it->second.update.endpoints.reset();
  it->second.update.resolution_note =
      absl::StrCat("EDS resource ", name, " does not exist");
  MaybeReportUpdate();
}

void XdsDependencyManager::OnDnsResult(const std::string& dns_name,
                                       Resolver::Result result) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] received DNS update: %s", this,
            dns_name.c_str());
  }
  if (xds_client_ == nullptr) return;
  auto it = dns_resolvers_.find(dns_name);
  if (it == dns_resolvers_.end()) return;
  // Convert resolver result to EDS update.
  XdsEndpointResource::Priority::Locality locality;
  locality.name = MakeRefCounted<XdsLocalityName>("", "", "");
  locality.lb_weight = 1;
  if (result.addresses.ok()) {
    locality.endpoints = std::move(*result.addresses);
    it->second.update.resolution_note = std::move(result.resolution_note);
  } else if (result.resolution_note.empty()) {
    it->second.update.resolution_note =
        absl::StrCat("DNS resolution failed for ", dns_name, ": ",
                     result.addresses.status().ToString());
  }
  XdsEndpointResource::Priority priority;
  priority.localities.emplace(locality.name.get(), std::move(locality));
  auto resource = std::make_shared<XdsEndpointResource>();
  resource->priorities.emplace_back(std::move(priority));
  it->second.update.endpoints = std::move(resource);
  MaybeReportUpdate();
}

std::set<std::string> XdsDependencyManager::GetClustersFromRouteConfig(
    const XdsRouteConfigResource& route_config) const {
  std::set<std::string> clusters;
  for (auto& route : current_virtual_host_->routes) {
    auto* route_action =
        absl::get_if<XdsRouteConfigResource::Route::RouteAction>(&route.action);
    if (route_action == nullptr) continue;
    Match(
        route_action->action,
        // cluster name
        [&](const XdsRouteConfigResource::Route::RouteAction::ClusterName&
                cluster_name) { clusters.insert(cluster_name.cluster_name); },
        // WeightedClusters
        [&](const std::vector<
            XdsRouteConfigResource::Route::RouteAction::ClusterWeight>&
                weighted_clusters) {
          for (const auto& weighted_cluster : weighted_clusters) {
            clusters.insert(weighted_cluster.name);
          }
        },
        // ClusterSpecifierPlugin
        [&](const XdsRouteConfigResource::Route::RouteAction::
                ClusterSpecifierPluginName&) {
          // Clusters are determined dynamically in this case, so we
          // can't add any clusters here.
        });
  }
  return clusters;
}

absl::StatusOr<bool> XdsDependencyManager::PopulateClusterConfigList(
    const std::string& name,
    std::vector<XdsConfig::ClusterConfig>* cluster_list, int depth,
    std::set<std::string>* clusters_seen,
    std::set<std::string>* eds_resources_seen) {
  if (depth == kMaxXdsAggregateClusterRecursionDepth) {
    return absl::FailedPreconditionError(
        "aggregate cluster graph exceeds max depth");
  }
  // Don't process the cluster again if we've already seen it in some
  // other branch of the recursion tree.
  if (!clusters_seen->insert(name).second) return true;
  auto& state = cluster_watchers_[name];
  // Create a new watcher if needed.
  if (state.watcher == nullptr) {
    auto watcher = MakeRefCounted<ClusterWatcher>(Ref(), name);
    if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
      gpr_log(GPR_INFO,
              "[XdsDependencyManager %p] starting watch for cluster %s", this,
              name.c_str());
    }
    state.watcher = watcher.get();
    XdsClusterResourceType::StartWatch(xds_client_.get(), name,
                                       std::move(watcher));
    return false;
  }
  // If there was an error fetching the CDS resource, report the error.
  if (!state.update.ok()) return state.update.status();
  // If we don't have the resource yet, we can't return a config yet.
  if (*state.update == nullptr) return false;
  // We have the resource, so add an entry to the map.
  cluster_list->emplace_back();
  auto& entry = cluster_list->back();
  entry.cluster_name = name;
  entry.cluster = *state.update;
  // Populate endpoint info based on cluster type.
  return Match(
      (*state.update)->type,
      // EDS cluster.
      [&](const XdsClusterResource::Eds& eds) -> absl::StatusOr<bool> {
        const std::string& eds_resource_name =
            eds.eds_service_name.empty() ? name : eds.eds_service_name;
        eds_resources_seen->insert(eds_resource_name);
        // Start EDS watch if needed.
        auto& eds_state = endpoint_watchers_[eds_resource_name];
        if (eds_state.watcher == nullptr) {
          if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
            gpr_log(GPR_INFO,
                    "[XdsDependencyManager %p] starting watch for endpoint %s",
                    this, eds_resource_name.c_str());
          }
          auto watcher =
              MakeRefCounted<EndpointWatcher>(Ref(), eds_resource_name);
          eds_state.watcher = watcher.get();
          XdsEndpointResourceType::StartWatch(
              xds_client_.get(), eds_resource_name, std::move(watcher));
          return false;
        }
        // Check if EDS resource has been returned.
        if (eds_state.update.endpoints == nullptr &&
            eds_state.update.resolution_note.empty()) {
          return false;
        }
        // Populate endpoint info.
        entry.endpoints = eds_state.update.endpoints;
        entry.resolution_note = eds_state.update.resolution_note;
        return true;
      },
      // LOGICAL_DNS cluster.
      [&](const XdsClusterResource::LogicalDns& logical_dns)
          -> absl::StatusOr<bool> {
        // FIXME: implement
        return true;
      },
      // Aggregate cluster.  Recursively expand to child clusters.
      [&](const XdsClusterResource::Aggregate& aggregate)
          -> absl::StatusOr<bool> {
        bool missing_cluster = false;
        for (const std::string& child_name :
             aggregate.prioritized_cluster_names) {
          auto result =
              PopulateClusterConfigList(child_name, cluster_list, depth + 1,
                                        clusters_seen, eds_resources_seen);
          if (!result.ok()) return result;
          if (!*result) missing_cluster = true;
        }
        return !missing_cluster;
      });
}

RefCountedPtr<XdsDependencyManager::ClusterSubscription>
XdsDependencyManager::GetClusterSubscription(std::string cluster_name) {
  auto it = cluster_subscriptions_.find(cluster_name);
  if (it != cluster_subscriptions_.end()) {
    auto subscription = it->second->RefIfNonZero();
    if (subscription != nullptr) return subscription;
  }
  auto subscription = MakeRefCounted<ClusterSubscription>(cluster_name, Ref());
  cluster_subscriptions_.emplace(std::move(cluster_name),
                                 subscription->WeakRef());
  MaybeReportUpdate();  // Trigger CDS watch.
  return subscription;
}

void XdsDependencyManager::OnClusterSubscriptionUnref(
    std::string cluster_name, ClusterSubscription* subscription) {
  auto it = cluster_subscriptions_.find(cluster_name);
  // Shouldn't happen, but ignore if it does.
  if (it == cluster_subscriptions_.end()) return;
  // Do nothing if the subscription has already been replaced.
  if (it->second != subscription) return;
  // Remove the entry.
  cluster_subscriptions_.erase(it);
  // If this cluster is also subscribed to by virtue of being
  // referenced in the route config, then we don't need to generate a
  // new update.
  if (clusters_from_route_config_.find(cluster_name) !=
      clusters_from_route_config_.end()) {
    return;
  }
  // Return an update without this cluster.
  MaybeReportUpdate();
}

void XdsDependencyManager::MaybeReportUpdate() {
  // Populate Listener and RouteConfig fields.
  if (current_virtual_host_ == nullptr) return;
  auto config = MakeRefCounted<XdsConfig>();
  config->listener = current_listener_;
  config->route_config = current_route_config_;
  config->virtual_host = current_virtual_host_;
  // Determine the set of clusters we should be watching.
  // FIXME: change this to use absl::string_view instead
  std::set<std::string> clusters_to_watch;
  for (const std::string& cluster : clusters_from_route_config_) {
    clusters_to_watch.insert(cluster);
  }
  for (const auto& p : cluster_subscriptions_) {
    clusters_to_watch.insert(p.first);
  }
  // Populate Cluster map.
  // We traverse the entire graph even if we don't yet have all of the
  // resources we need to ensure that the right set of watches are active.
  std::set<std::string> clusters_seen;
  std::set<std::string> eds_resources_seen;
  bool missing_data = false;
  for (const std::string& cluster : clusters_to_watch) {
    auto& cluster_list = config->clusters[cluster];
    cluster_list.emplace();
    auto result = PopulateClusterConfigList(
        cluster, &*cluster_list, 0, &clusters_seen, &eds_resources_seen);
    if (!result.ok()) {
      cluster_list = result.status();
    } else if (!*result) {
      missing_data = true;
    } else if (cluster_list->empty()) {
      cluster_list = absl::UnavailableError(
          absl::StrCat("aggregate cluster dependency graph for ", cluster,
                       " has no leaf clusters"));
    }
  }
  // Remove entries in cluster_watchers_ for any clusters not in clusters_seen.
  for (auto it = cluster_watchers_.begin(); it != cluster_watchers_.end();) {
    const std::string& cluster_name = it->first;
    if (clusters_seen.find(cluster_name) != clusters_seen.end()) {
      ++it;
      continue;
    }
    if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
      gpr_log(GPR_INFO,
              "[XdsDependencyManager %p] cancelling watch for cluster %s", this,
              cluster_name.c_str());
    }
    XdsClusterResourceType::CancelWatch(xds_client_.get(), cluster_name,
                                        it->second.watcher,
                                        /*delay_unsubscription=*/false);
    it = cluster_watchers_.erase(it);
  }
  // Remove entries in endpoint_watchers_ for any EDS resources not in
  // eds_resources_seen.
  for (auto it = endpoint_watchers_.begin(); it != endpoint_watchers_.end();) {
    const std::string& eds_resource_name = it->first;
    if (eds_resources_seen.find(eds_resource_name) !=
        eds_resources_seen.end()) {
      ++it;
      continue;
    }
    if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
      gpr_log(GPR_INFO,
              "[XdsDependencyManager %p] cancelling watch for EDS resource %s",
              this, eds_resource_name.c_str());
    }
    XdsEndpointResourceType::CancelWatch(xds_client_.get(), eds_resource_name,
                                         it->second.watcher,
                                         /*delay_unsubscription=*/false);
    it = endpoint_watchers_.erase(it);
  }
  // If we have all the data we need, then send an update.
  if (GRPC_TRACE_FLAG_ENABLED(grpc_xds_resolver_trace)) {
    gpr_log(GPR_INFO, "[XdsDependencyManager %p] %sreturning updated config",
            this, missing_data ? "NOT " : "");
  }
  if (!missing_data) watcher_->OnUpdate(std::move(config));
}

}  // namespace grpc_core

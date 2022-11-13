//
// Copyright 2022 gRPC authors.
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

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/strip.h"

#include <grpc/support/log.h>

#include "src/core/ext/xds/xds_bootstrap_grpc.h"
#include "src/core/ext/xds/xds_client.h"
#include "src/core/ext/xds/xds_cluster.h"
#include "src/core/ext/xds/xds_endpoint.h"
#include "src/core/ext/xds/xds_listener.h"
#include "src/core/ext/xds/xds_route_config.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/libfuzzer/libfuzzer_macro.h"
#include "src/proto/grpc/testing/xds/v3/cluster.pb.h"
#include "src/proto/grpc/testing/xds/v3/discovery.pb.h"
#include "src/proto/grpc/testing/xds/v3/endpoint.pb.h"
#include "src/proto/grpc/testing/xds/v3/http_connection_manager.pb.h"
#include "src/proto/grpc/testing/xds/v3/listener.pb.h"
#include "src/proto/grpc/testing/xds/v3/route.pb.h"
#include "src/proto/grpc/testing/xds/v3/router.pb.h"
#include "test/core/xds/xds_client_fuzzer.pb.h"
#include "test/core/xds/xds_transport_fake.h"

namespace grpc_core {

TraceFlag xds_client_fuzzer_trace(true, "xds_client_fuzzer");

class Fuzzer {
 public:
  explicit Fuzzer(absl::string_view bootstrap_json) {
    auto bootstrap = GrpcXdsBootstrap::Create(bootstrap_json);
    if (!bootstrap.ok()) {
      gpr_log(GPR_ERROR, "error creating bootstrap: %s",
              bootstrap.status().ToString().c_str());
      // Leave xds_client_ unset, so Act() and EndAct() will be no-ops.
      return;
    }
    auto transport_factory = MakeOrphanable<FakeXdsTransportFactory>();
    transport_factory->SetAutoCompleteMessagesFromClient(false);
    transport_factory_ = transport_factory.get();
    xds_client_ = MakeRefCounted<XdsClient>(std::move(*bootstrap),
                                            std::move(transport_factory));
  }

  void Act(const xds_client_fuzzer::Action& action) {
    if (xds_client_ == nullptr) return;
    switch (action.action_type_case()) {
      case xds_client_fuzzer::Action::kStartWatch:
        switch (action.start_watch().resource_type().resource_type_case()) {
          case xds_client_fuzzer::ResourceType::kListener:
            StartWatch(&listener_watchers_,
                       action.start_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::kRouteConfig:
            StartWatch(&route_config_watchers_,
                       action.start_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::kCluster:
            StartWatch(&cluster_watchers_,
                       action.start_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::kEndpoint:
            StartWatch(&endpoint_watchers_,
                       action.start_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::RESOURCE_TYPE_NOT_SET:
            break;
        }
        break;
      case xds_client_fuzzer::Action::kStopWatch:
        switch (action.stop_watch().resource_type().resource_type_case()) {
          case xds_client_fuzzer::ResourceType::kListener:
            StopWatch(&listener_watchers_, action.stop_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::kRouteConfig:
            StopWatch(&route_config_watchers_,
                      action.stop_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::kCluster:
            StopWatch(&cluster_watchers_, action.stop_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::kEndpoint:
            StopWatch(&endpoint_watchers_, action.stop_watch().resource_name());
            break;
          case xds_client_fuzzer::ResourceType::RESOURCE_TYPE_NOT_SET:
            break;
        }
        break;
      case xds_client_fuzzer::Action::kDumpCsdsData:
        xds_client_->DumpClientConfigBinary();
        break;
      case xds_client_fuzzer::Action::kTriggerConnectionFailure:
        TriggerConnectionFailure(
            action.trigger_connection_failure().authority(),
            ToAbslStatus(action.trigger_connection_failure().status()));
        break;
      case xds_client_fuzzer::Action::kReadMessageFromClient:
        ReadMessageFromClient(action.read_message_from_client().stream_id(),
                              action.read_message_from_client().ok());
        break;
      case xds_client_fuzzer::Action::kSendMessageToClient:
        SendMessageToClient(action.send_message_to_client().stream_id(),
                            action.send_message_to_client().response());
        break;
      case xds_client_fuzzer::Action::kSendStatusToClient:
        SendStatusToClient(
            action.send_status_to_client().stream_id(),
            ToAbslStatus(action.send_status_to_client().status()));
        break;
      case xds_client_fuzzer::Action::ACTION_TYPE_NOT_SET:
        break;
    }
  }

  void EndAct(const xds_client_fuzzer::EndAction& end_action) {
    if (xds_client_ == nullptr) return;
    switch (end_action.action_type_case()) {
      case xds_client_fuzzer::EndAction::kSendKnownGoodResources:
        SendKnownGoodResources(end_action.send_known_good_resources());
        break;
      case xds_client_fuzzer::EndAction::ACTION_TYPE_NOT_SET:
        break;
    }
  }

 private:
  template <typename ResourceTypeArg>
  class Watcher : public ResourceTypeArg::WatcherInterface {
   public:
    using ResourceType = ResourceTypeArg;

    enum class Event { kChanged, kError, kDoesNotExist };

    explicit Watcher(std::string resource_name)
        : resource_name_(std::move(resource_name)) {}

    ~Watcher() override {
      auto event_queue_string = [&](const std::vector<Event>& event_queue) {
        std::vector<absl::string_view> parts;
        for (Event event : event_queue) {
          switch (event) {
            case Event::kChanged:
              parts.push_back("CHANGED");
              break;
            case Event::kError:
              parts.push_back("ERROR");
              break;
            case Event::kDoesNotExist:
              parts.push_back("DOES_NOT_EXIST");
              break;
          }
        }
        return absl::StrJoin(parts, " ");
      };
      std::string events_expected_str = event_queue_string(events_expected_);
      std::string events_seen_str = event_queue_string(events_seen_);
      gpr_log(GPR_INFO, "EXPECTED EVENTS: %s", events_expected_str.c_str());
      gpr_log(GPR_INFO, "SEEN EVENTS    : %s", events_seen_str.c_str());
      GPR_ASSERT(events_seen_str == events_expected_str);
    }

    void AddExpectedEvent(Event event) {
      if (event != Event::kDoesNotExist || events_expected_.empty() ||
          events_expected_.back() != Event::kDoesNotExist) {
        events_expected_.push_back(event);
      }
    }

    // Returns true if the resource is different from the previous value.
    bool SetLastSeen(const XdsResourceType::ResourceData* resource) {
      if (last_seen_ != nullptr &&
          ResourceType::Get()->ResourcesEqual(last_seen_.get(), resource)) {
        return false;
      }
      last_seen_ = ResourceType::Get()->CopyResource(resource);
      return true;
    }

   private:
    void OnResourceChanged(
        typename ResourceType::ResourceType resource) override {
      gpr_log(GPR_INFO, "==> OnResourceChanged(%s %s): %s",
              std::string(ResourceType::Get()->type_url()).c_str(),
              resource_name_.c_str(), resource.ToString().c_str());
      events_seen_.push_back(Event::kChanged);
    }

    void OnError(absl::Status status) override {
      gpr_log(GPR_INFO, "==> OnError(%s %s): %s",
              std::string(ResourceType::Get()->type_url()).c_str(),
              resource_name_.c_str(), status.ToString().c_str());
      events_seen_.push_back(Event::kError);
    }

    void OnResourceDoesNotExist() override {
      gpr_log(GPR_INFO, "==> OnResourceDoesNotExist(%s %s)",
              std::string(ResourceType::Get()->type_url()).c_str(),
              resource_name_.c_str());
      events_seen_.push_back(Event::kDoesNotExist);
    }

    std::string resource_name_;
    std::vector<Event> events_seen_;
    std::vector<Event> events_expected_;
    std::unique_ptr<XdsResourceType::ResourceData> last_seen_;
  };

  using ListenerWatcher = Watcher<XdsListenerResourceType>;
  using RouteConfigWatcher = Watcher<XdsRouteConfigResourceType>;
  using ClusterWatcher = Watcher<XdsClusterResourceType>;
  using EndpointWatcher = Watcher<XdsEndpointResourceType>;

  template <typename WatcherType>
  void StartWatch(std::map<std::string, std::set<WatcherType*>>* watchers,
                  std::string resource_name) {
    gpr_log(GPR_INFO, "### StartWatch(%s %s)",
            std::string(WatcherType::ResourceType::Get()->type_url()).c_str(),
            resource_name.c_str());
    auto watcher = MakeRefCounted<WatcherType>(resource_name);
    (*watchers)[resource_name].insert(watcher.get());
    WatcherType::ResourceType::Get()->StartWatch(
        xds_client_.get(), resource_name, std::move(watcher));
  }

  template <typename WatcherType>
  void StopWatch(std::map<std::string, std::set<WatcherType*>>* watchers,
                 std::string resource_name) {
    gpr_log(GPR_INFO, "### StopWatch(%s %s)",
            std::string(WatcherType::ResourceType::Get()->type_url()).c_str(),
            resource_name.c_str());
    auto& watchers_set = (*watchers)[resource_name];
    auto it = watchers_set.begin();
    if (it == watchers_set.end()) return;
    WatcherType::ResourceType::Get()->CancelWatch(xds_client_.get(),
                                                  resource_name, *it);
    watchers_set.erase(it);
  }

  static absl::Status ToAbslStatus(const xds_client_fuzzer::Status& status) {
    return absl::Status(static_cast<absl::StatusCode>(status.code()),
                        status.message());
  }

  const XdsBootstrap::XdsServer* GetServer(const std::string& authority) {
    const GrpcXdsBootstrap& bootstrap =
        static_cast<const GrpcXdsBootstrap&>(xds_client_->bootstrap());
    if (authority.empty()) return &bootstrap.server();
    const auto* authority_entry =
        static_cast<const GrpcXdsBootstrap::GrpcAuthority*>(
            bootstrap.LookupAuthority(authority));
    if (authority_entry == nullptr) return nullptr;
    if (authority_entry->server() != nullptr) return authority_entry->server();
    return &bootstrap.server();
  }

  void TriggerConnectionFailure(const std::string& authority,
                                absl::Status status) {
    gpr_log(GPR_INFO, "### TriggerConnectionFailure(%s): %s", authority.c_str(),
            status.ToString().c_str());
    const auto* xds_server = GetServer(authority);
    if (xds_server == nullptr) return;
    transport_factory_->TriggerConnectionFailure(*xds_server,
                                                 std::move(status));
  }

  // Records expectations based on the resources being sent to the client.
  template <typename WatcherType>
  void RecordReceivedResources(
      const XdsBootstrap::XdsServer& xds_server,
      const envoy::service::discovery::v3::DiscoveryResponse& response,
      std::map<std::string, std::set<WatcherType*>>* watchers) {
    std::set<std::string> resources_seen;
    upb::DefPool def_pool;
    upb::Arena arena;
    const XdsResourceType::DecodeContext context = {
        xds_client_.get(), xds_server, &xds_client_fuzzer_trace, def_pool.ptr(),
        arena.ptr()};
    for (const auto& any : response.resources()) {
      // If the resource type of this resource does not match the type
      // of the response, skip it.
      if (any.type_url() != response.type_url()) continue;
      // Attempt to decode the resource.
      auto decode_result =
          WatcherType::ResourceType::Get()->Decode(context, any.value());
      // If we couldn't even decode the resource's name, then there's
      // nothing to record.
      if (!decode_result.name.has_value()) continue;
      // If we found the name, then check if there are currently
      // watchers for this resource.
      auto it = watchers->find(*decode_result.name);
      if (it == watchers->end()) continue;
      // Set expectations on each watcher.
      for (auto* watcher : it->second) {
        // If we were not able to decide the resource, the watcher
        // should see an error.
        // Otherwise, if the resource has changed, we should see an update.
        if (!decode_result.resource.ok()) {
          watcher->AddExpectedEvent(WatcherType::Event::kError);
        } else if (watcher->SetLastSeen((*decode_result.resource).get())) {
          watcher->AddExpectedEvent(WatcherType::Event::kChanged);
        }
      }
      // If this resource type requires all resources to be present in
      // every response, then record the resource names we've seen.
      if (WatcherType::ResourceType::Get()->AllResourcesRequiredInSotW()) {
        resources_seen.insert(*decode_result.name);
      }
    }
    // If this resource type requires all resources to be present in
    // every response, then record a does-not-exist for any resource
    // type we haven't seen.
    if (!xds_server.IgnoreResourceDeletion() &&
        WatcherType::ResourceType::Get()->AllResourcesRequiredInSotW()) {
      for (const auto& p : *watchers) {
        if (resources_seen.find(p.first) == resources_seen.end()) {
          for (auto* watcher : p.second) {
            watcher->AddExpectedEvent(WatcherType::Event::kDoesNotExist);
          }
        }
      }
    }
  }

  static const char* StreamIdMethod(
      const xds_client_fuzzer::StreamId& stream_id) {
    switch (stream_id.method_case()) {
      case xds_client_fuzzer::StreamId::kAds:
        return FakeXdsTransportFactory::kAdsMethod;
      case xds_client_fuzzer::StreamId::kLrs:
        return FakeXdsTransportFactory::kLrsMethod;
      case xds_client_fuzzer::StreamId::METHOD_NOT_SET:
        return nullptr;
    }
  }

  RefCountedPtr<FakeXdsTransportFactory::FakeStreamingCall> GetStream(
      const xds_client_fuzzer::StreamId& stream_id) {
    const auto* xds_server = GetServer(stream_id.authority());
    if (xds_server == nullptr) return nullptr;
    const char* method = StreamIdMethod(stream_id);
    if (method == nullptr) return nullptr;
    return transport_factory_->WaitForStream(*xds_server, method,
                                             absl::ZeroDuration());
  }

  static std::string StreamIdString(
      const xds_client_fuzzer::StreamId& stream_id) {
    return absl::StrCat("{authority=\"", stream_id.authority(),
                        "\", method=", StreamIdMethod(stream_id), "}");
  }

  void ReadMessageFromClient(const xds_client_fuzzer::StreamId& stream_id,
                             bool ok) {
    gpr_log(GPR_INFO, "### ReadMessageFromClient(%s): %s",
            StreamIdString(stream_id).c_str(), ok ? "true" : "false");
    auto stream = GetStream(stream_id);
    if (stream == nullptr) return;
    gpr_log(GPR_INFO, "    stream=%p", stream.get());
    auto message = stream->WaitForMessageFromClient(absl::ZeroDuration());
    if (message.has_value()) {
      gpr_log(GPR_INFO, "    completing send_message");
      stream->CompleteSendMessageFromClient(ok);
    }
  }

  void SendMessageToClient(
      const xds_client_fuzzer::StreamId& stream_id,
      const envoy::service::discovery::v3::DiscoveryResponse& response) {
    gpr_log(GPR_INFO, "### SendMessageToClient(%s)",
            StreamIdString(stream_id).c_str());
    auto stream = GetStream(stream_id);
    if (stream == nullptr) return;
    gpr_log(GPR_INFO, "    stream=%p", stream.get());
    const auto* xds_server = GetServer(stream_id.authority());
    GPR_ASSERT(xds_server != nullptr);
    // Record expected events for watchers, if any.
    absl::string_view type_url =
        absl::StripPrefix(response.type_url(), "type.googleapis.com/");
    if (type_url == ListenerWatcher::ResourceType::Get()->type_url()) {
      RecordReceivedResources(*xds_server, response, &listener_watchers_);
    } else if (type_url ==
               RouteConfigWatcher::ResourceType::Get()->type_url()) {
      RecordReceivedResources(*xds_server, response, &route_config_watchers_);
    } else if (type_url == ClusterWatcher::ResourceType::Get()->type_url()) {
      RecordReceivedResources(*xds_server, response, &cluster_watchers_);
    } else if (type_url == EndpointWatcher::ResourceType::Get()->type_url()) {
      RecordReceivedResources(*xds_server, response, &endpoint_watchers_);
    }
    // Now send the message to the client.
    stream->SendMessageToClient(response.SerializeAsString());
  }

  void SendStatusToClient(const xds_client_fuzzer::StreamId& stream_id,
                          absl::Status status) {
    gpr_log(GPR_INFO, "### SendStatusToClient(%s): %s",
            StreamIdString(stream_id).c_str(), status.ToString().c_str());
    auto stream = GetStream(stream_id);
    if (stream == nullptr) return;
    gpr_log(GPR_INFO, "    stream=%p", stream.get());
    stream->MaybeSendStatusToClient(std::move(status));
  }

  void AddKnownGoodListenerResources(
      const xds_client_fuzzer::SendKnownGoodResources&
          send_known_good_resources,
      envoy::service::discovery::v3::DiscoveryResponse* response) {
    response->set_type_url(absl::StrCat(
        "type.googleapis.com/", XdsListenerResourceType::Get()->type_url()));
    envoy::config::listener::v3::Listener listener;
    envoy::extensions::filters::network::http_connection_manager::v3::
        HttpConnectionManager hcm;
    auto* rds = hcm.mutable_rds();
    rds->set_route_config_name("route_config_name");
    rds->mutable_config_source()->mutable_self();
    auto* http_filter = hcm.add_http_filters();
    http_filter->set_name("router");
    http_filter->mutable_typed_config()->PackFrom(
        envoy::extensions::filters::http::router::v3::Router());
    listener.mutable_api_listener()->mutable_api_listener()->PackFrom(hcm);
    for (const std::string& resource_name :
         send_known_good_resources.resource_names()) {
      listener.set_name(resource_name);
      response->add_resources()->PackFrom(listener);
    }
  }

  void AddKnownGoodRouteConfigResources(
      const xds_client_fuzzer::SendKnownGoodResources&
          send_known_good_resources,
      envoy::service::discovery::v3::DiscoveryResponse* response) {
    response->set_type_url(absl::StrCat(
        "type.googleapis.com/", XdsRouteConfigResourceType::Get()->type_url()));
    envoy::config::route::v3::RouteConfiguration route_config;
    auto* vhost = route_config.add_virtual_hosts();
    vhost->add_domains("*");
    auto* route = vhost->add_routes();
    route->mutable_match()->set_prefix("");
    route->mutable_route()->set_cluster("cluster1");
    for (const std::string& resource_name :
         send_known_good_resources.resource_names()) {
      route_config.set_name(resource_name);
      response->add_resources()->PackFrom(route_config);
    }
  }

  void AddKnownGoodClusterResources(
      const xds_client_fuzzer::SendKnownGoodResources&
          send_known_good_resources,
      envoy::service::discovery::v3::DiscoveryResponse* response) {
    response->set_type_url(absl::StrCat(
        "type.googleapis.com/", XdsClusterResourceType::Get()->type_url()));
    envoy::config::cluster::v3::Cluster cluster;
    cluster.set_type(cluster.EDS);
    auto* eds_config = cluster.mutable_eds_cluster_config();
    eds_config->mutable_eds_config()->mutable_self();
    eds_config->set_service_name("endpoint1");
    for (const std::string& resource_name :
         send_known_good_resources.resource_names()) {
      cluster.set_name(resource_name);
      response->add_resources()->PackFrom(cluster);
    }
  }

  void AddKnownGoodEndpointResources(
      const xds_client_fuzzer::SendKnownGoodResources&
          send_known_good_resources,
      envoy::service::discovery::v3::DiscoveryResponse* response) {
    response->set_type_url(absl::StrCat(
        "type.googleapis.com/", XdsEndpointResourceType::Get()->type_url()));
    envoy::config::endpoint::v3::ClusterLoadAssignment endpoint;
    auto* endpoints = endpoint.add_endpoints();
    auto* locality = endpoints->mutable_locality();
    locality->set_region("region1");
    locality->set_zone("zone1");
    locality->set_sub_zone("sub_zone1");
    endpoints->mutable_load_balancing_weight()->set_value(1);
    auto* ep = endpoints->add_lb_endpoints();
    ep->mutable_load_balancing_weight()->set_value(1);
    auto* socket_addr =
        ep->mutable_endpoint()->mutable_address()->mutable_socket_address();
    socket_addr->set_address("127.0.0.1");
    socket_addr->set_port_value(443);
    for (const std::string& resource_name :
         send_known_good_resources.resource_names()) {
      endpoint.set_cluster_name(resource_name);
      response->add_resources()->PackFrom(endpoint);
    }
  }

  void SendKnownGoodResources(const xds_client_fuzzer::SendKnownGoodResources&
                                  send_known_good_resources) {
    gpr_log(
        GPR_INFO, "### SendKnownGoodResources(%s): %s",
        StreamIdString(send_known_good_resources.stream_id()).c_str(),
        absl::StrJoin(send_known_good_resources.resource_names(), " ").c_str());
    envoy::service::discovery::v3::DiscoveryResponse response;
    response.set_version_info("end_action");
    response.set_nonce("end_action");
    switch (send_known_good_resources.resource_type().resource_type_case()) {
      case xds_client_fuzzer::ResourceType::kListener:
        gpr_log(GPR_INFO, "    Listener");
        AddKnownGoodListenerResources(send_known_good_resources, &response);
        break;
      case xds_client_fuzzer::ResourceType::kRouteConfig:
        gpr_log(GPR_INFO, "    RouteConfig");
        AddKnownGoodRouteConfigResources(send_known_good_resources, &response);
        break;
      case xds_client_fuzzer::ResourceType::kCluster:
        gpr_log(GPR_INFO, "    Cluster");
        AddKnownGoodClusterResources(send_known_good_resources, &response);
        break;
      case xds_client_fuzzer::ResourceType::kEndpoint:
        gpr_log(GPR_INFO, "    Endpoint");
        AddKnownGoodEndpointResources(send_known_good_resources, &response);
        break;
      case xds_client_fuzzer::ResourceType::RESOURCE_TYPE_NOT_SET:
        return;
    }
    SendMessageToClient(send_known_good_resources.stream_id(), response);
  }

  RefCountedPtr<XdsClient> xds_client_;
  FakeXdsTransportFactory* transport_factory_;

  // Maps of currently active watchers for each resource type, keyed by
  // resource name.
  std::map<std::string, std::set<ListenerWatcher*>> listener_watchers_;
  std::map<std::string, std::set<RouteConfigWatcher*>> route_config_watchers_;
  std::map<std::string, std::set<ClusterWatcher*>> cluster_watchers_;
  std::map<std::string, std::set<EndpointWatcher*>> endpoint_watchers_;
};

}  // namespace grpc_core

bool squelch = true;

DEFINE_PROTO_FUZZER(const xds_client_fuzzer::Message& message) {
  grpc_init();
  grpc_core::Fuzzer fuzzer(message.bootstrap());
  for (const auto& action : message.actions()) {
    fuzzer.Act(action);
  }
  for (const auto& end_action : message.end_actions()) {
    fuzzer.EndAct(end_action);
  }
  grpc_shutdown();
}

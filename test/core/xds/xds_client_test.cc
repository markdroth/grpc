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

#include "src/core/ext/xds/xds_client.h"

#include <deque>
#include <map>
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/core/ext/xds/xds_bootstrap.h"
#include "src/core/ext/xds/xds_resource_type_impl.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/json/json.h"
#include "src/core/lib/json/json_object_loader.h"
#include "src/proto/grpc/testing/xds/v3/discovery.grpc.pb.h"
#include "test/core/util/test_config.h"
#include "test/core/xds/xds_transport_fake.h"

using envoy::service::discovery::v3::DiscoveryRequest;
using envoy::service::discovery::v3::DiscoveryResponse;

namespace grpc_core {
namespace testing {
namespace {

class XdsClientTest : public ::testing::Test {
 protected:
  // A fake bootstrap implementation that allows tests to populate the
  // fields however they want.
  class FakeXdsBootstrap : public XdsBootstrap {
   public:
    class Builder {
     public:
      Builder() {
        server_.server_uri = "default_xds_server";
        server_.server_features.insert(
            std::string(XdsServer::kServerFeatureXdsV3));
        node_.emplace();
        node_->id = "xds_client_test";
      }

      Builder& set_server(XdsServer server) {
        server_ = std::move(server);
        return *this;
      }
      Builder& set_use_v2() {
        server_.server_features.erase(
            std::string(XdsServer::kServerFeatureXdsV3));
        return *this;
      }
      Builder& set_node(absl::optional<Node> node) {
        node_ = std::move(node);
        return *this;
      }
      Builder& AddAuthority(std::string name, Authority authority) {
        authorities_[std::move(name)] = std::move(authority);
        return *this;
      }
      std::unique_ptr<XdsBootstrap> Build() {
        auto bootstrap = absl::make_unique<FakeXdsBootstrap>();
        bootstrap->server_ = std::move(server_);
        bootstrap->node_ = std::move(node_);
        bootstrap->authorities_ = std::move(authorities_);
        return bootstrap;
      }

     private:
      XdsServer server_;
      absl::optional<Node> node_;
      std::map<std::string, Authority> authorities_;
    };

    std::string ToString() const override { return "<fake>"; }

    const XdsServer& server() const override { return server_; }
    const Node* node() const override { return &node_.value(); }
    const std::map<std::string, Authority>& authorities() const override {
      return authorities_;
    }

   private:
    XdsServer server_;
    absl::optional<Node> node_;
    std::map<std::string, Authority> authorities_;
  };

  // A fake xDS resource type called "test.v3.foo".
  // The payload is a JSON string that parses to an XdsFooResource struct.
  struct XdsFooResource {
    std::string name;
    uint32_t value;

    bool operator==(const XdsFooResource& other) const {
      return name == other.name && value == other.value;
    }

    std::string AsJsonString() const {
      return absl::StrCat("{\"name\":\"", name, "\",\"value\":", value, "}");
    }

    static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
      static const auto* loader = JsonObjectLoader<XdsFooResource>()
                                      .Field("name", &XdsFooResource::name)
                                      .Field("value", &XdsFooResource::value)
                                      .Finish();
      return loader;
    }
  };
  class XdsFooResourceType
      : public XdsResourceTypeImpl<XdsFooResourceType, XdsFooResource> {
   public:
    absl::string_view type_url() const override { return "test.v3.foo"; }
    absl::string_view v2_type_url() const override { return "test.v2.foo"; }
    absl::StatusOr<DecodeResult> Decode(
        const XdsResourceType::DecodeContext& /*context*/,
        absl::string_view serialized_resource, bool /*is_v2*/) const override {
      auto json = Json::Parse(serialized_resource);
      if (!json.ok()) return json.status();
      absl::StatusOr<XdsFooResource> foo = LoadFromJson<XdsFooResource>(*json);
      DecodeResult result;
      if (!foo.ok()) {
        auto it = json->object_value().find("name");
        if (it == json->object_value().end()) {
          return absl::InvalidArgumentError(
              "cannot determine name for invalid resource");
        }
        result.name = it->second.string_value();
        result.resource = foo.status();
      } else {
        result.name = foo->name;
        auto resource = absl::make_unique<ResourceDataSubclass>();
        resource->resource = std::move(*foo);
        result.resource = std::move(resource);
      }
      return std::move(result);
    }
    void InitUpbSymtab(upb_DefPool* /*symtab*/) const override {}

    static google::protobuf::Any EncodeAsAny(const XdsFooResource& resource) {
      google::protobuf::Any any;
      any.set_type_url(absl::StrCat("type.googleapis.com/", Get()->type_url()));
      any.set_value(resource.AsJsonString());
      return any;
    }
  };

  // A watcher implementation that queues delivered watches.
  // FIXME: need to change the API here such that all 3 types of events
  // are returned in a single queue, so that we can tell the order in
  // which they were received
  class FooWatcher : public XdsFooResourceType::WatcherInterface {
   public:
    absl::optional<XdsFooResource> WaitForNextResource(
        absl::Duration timeout = absl::Seconds(1)) {
      MutexLock lock(&mu_);
      while (queue_.empty()) {
        if (cv_.WaitWithTimeout(&mu_, timeout * grpc_test_slowdown_factor())) {
          return absl::nullopt;
        }
      }
      XdsFooResource foo = std::move(queue_.front());
      queue_.pop_front();
      return foo;
    }

    bool HasError() {
      MutexLock lock(&mu_);
      return !error_queue_.empty();
    }

    absl::optional<absl::Status> WaitForNextError(
        absl::Duration timeout = absl::Seconds(1)) {
      MutexLock lock(&mu_);
      while (error_queue_.empty()) {
        if (cv_.WaitWithTimeout(&mu_, timeout * grpc_test_slowdown_factor())) {
          return absl::nullopt;
        }
      }
      absl::Status status = std::move(error_queue_.front());
      error_queue_.pop_front();
      return status;
    }

    bool WaitForDoesNotExist(absl::Duration timeout) {
      MutexLock lock(&mu_);
      while (!does_not_exist_) {
        if (cv_.WaitWithTimeout(&mu_, timeout * grpc_test_slowdown_factor())) {
          return false;
        }
      }
      does_not_exist_ = false;  // Reset in case we ask again later.
      return true;
    }

   private:
    void OnResourceChanged(XdsFooResource foo) override {
      MutexLock lock(&mu_);
      queue_.push_back(std::move(foo));
      cv_.Signal();
    }
    void OnError(absl::Status status) override {
      MutexLock lock(&mu_);
      error_queue_.push_back(std::move(status));
      cv_.Signal();
    }
    void OnResourceDoesNotExist() override {
      MutexLock lock(&mu_);
      does_not_exist_ = true;
      cv_.Signal();
    }

    Mutex mu_;
    CondVar cv_;
    std::deque<XdsFooResource> queue_ ABSL_GUARDED_BY(&mu_);
    std::deque<absl::Status> error_queue_ ABSL_GUARDED_BY(&mu_);
    bool does_not_exist_ ABSL_GUARDED_BY(&mu_) = false;
  };

  // A helper class to build and serialize a DiscoveryResponse.
  class ResponseBuilder {
   public:
    explicit ResponseBuilder(absl::string_view type_url) {
      response_.set_type_url(absl::StrCat("type.googleapis.com/", type_url));
    }

    ResponseBuilder& set_version_info(absl::string_view version_info) {
      response_.set_version_info(std::string(version_info));
      return *this;
    }
    ResponseBuilder& set_nonce(absl::string_view nonce) {
      response_.set_nonce(std::string(nonce));
      return *this;
    }

    ResponseBuilder& AddResource(const XdsFooResource& resource,
                                 bool in_resource_wrapper = false) {
      auto* res = response_.add_resources();
      *res = XdsFooResourceType::EncodeAsAny(resource);
      if (in_resource_wrapper) {
        envoy::service::discovery::v3::Resource resource_wrapper;
        resource_wrapper.set_name(resource.name);
        *resource_wrapper.mutable_resource() = std::move(*res);
        res->PackFrom(resource_wrapper);
      }
      return *this;
    }

    ResponseBuilder& AddInvalidResource(
        absl::string_view type_url, absl::string_view value,
        absl::string_view resource_wrapper_name = "") {
      auto* res = response_.add_resources();
      res->set_type_url(absl::StrCat("type.googleapis.com/", type_url));
      res->set_value(std::string(value));
      if (!resource_wrapper_name.empty()) {
        envoy::service::discovery::v3::Resource resource_wrapper;
        resource_wrapper.set_name(std::string(resource_wrapper_name));
        *resource_wrapper.mutable_resource() = std::move(*res);
        res->PackFrom(resource_wrapper);
      }
      return *this;
    }

    std::string Serialize() {
      std::string serialized_response;
      EXPECT_TRUE(response_.SerializeToString(&serialized_response));
      return serialized_response;
    }

   private:
    DiscoveryResponse response_;
  };

  // Sets transport_factory_ and initializes xds_client_ with the
  // specified bootstrap config.
  void InitXdsClient(
      FakeXdsBootstrap::Builder bootstrap_builder = FakeXdsBootstrap::Builder(),
      Duration resource_request_timeout = Duration::Seconds(15) *
                                          grpc_test_slowdown_factor()) {
    auto transport_factory = MakeOrphanable<FakeXdsTransportFactory>();
    transport_factory_ = transport_factory->Ref();
    xds_client_ = MakeRefCounted<XdsClient>(bootstrap_builder.Build(),
                                            std::move(transport_factory),
                                            resource_request_timeout);
  }

  // Starts a watch for the named resource.
  RefCountedPtr<FooWatcher> StartFooWatch(absl::string_view resource_name) {
    auto watcher = MakeRefCounted<FooWatcher>();
    XdsFooResourceType::StartWatch(xds_client_.get(), resource_name, watcher);
    return watcher;
  }

  // Cancels the specified watch.
  void CancelFooWatch(FooWatcher* watcher, absl::string_view resource_name,
                      bool delay_unsubscription = false) {
    XdsFooResourceType::CancelWatch(xds_client_.get(), resource_name, watcher,
                                    delay_unsubscription);
  }

  RefCountedPtr<FakeXdsTransportFactory::FakeStreamingCall> WaitForAdsStream(
      absl::Duration timeout = absl::Seconds(5)) {
    return transport_factory_->WaitForStream(
        xds_client_->bootstrap().server(), FakeXdsTransportFactory::kAdsMethod,
        timeout * grpc_test_slowdown_factor());
  }

  // Gets the latest request sent to the fake xDS server.
  absl::optional<DiscoveryRequest> WaitForRequest(
      FakeXdsTransportFactory::FakeStreamingCall* stream,
      absl::Duration timeout = absl::Seconds(1),
      SourceLocation location = SourceLocation()) {
    auto message =
        stream->WaitForMessageFromClient(timeout * grpc_test_slowdown_factor());
    if (!message.has_value()) return absl::nullopt;
    DiscoveryRequest request;
    bool success = request.ParseFromString(*message);
    EXPECT_TRUE(success) << "Failed to deserialize DiscoveryRequest at "
                         << location.file() << ":" << location.line();
    if (!success) return absl::nullopt;
    return std::move(request);
  }

  // Helper function to check the fields of a DiscoveryRequest.
  void CheckRequest(const DiscoveryRequest& request, absl::string_view type_url,
                    absl::string_view version_info,
                    absl::string_view response_nonce, absl::Status error_detail,
                    std::set<absl::string_view> resource_names,
                    SourceLocation location = SourceLocation()) {
    EXPECT_EQ(request.type_url(),
              absl::StrCat("type.googleapis.com/", type_url))
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.version_info(), version_info)
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.response_nonce(), response_nonce)
        << location.file() << ":" << location.line();
    if (error_detail.ok()) {
      EXPECT_FALSE(request.has_error_detail())
          << location.file() << ":" << location.line();
    } else {
      EXPECT_EQ(request.error_detail().code(),
                static_cast<int>(error_detail.code()))
          << location.file() << ":" << location.line();
      EXPECT_EQ(request.error_detail().message(), error_detail.message())
          << location.file() << ":" << location.line();
    }
    EXPECT_THAT(request.resource_names(),
                ::testing::UnorderedElementsAreArray(resource_names))
        << location.file() << ":" << location.line();
  }

  // Helper function to check the contents of the node message in a
  // request against the client's node info.
  void CheckRequestNode(const DiscoveryRequest& request,
                        bool check_build_version = false,
                        SourceLocation location = SourceLocation()) {
    // These fields come from the bootstrap config.
    EXPECT_EQ(request.node().id(), xds_client_->bootstrap().node()->id)
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.node().cluster(),
              xds_client_->bootstrap().node()->cluster)
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.node().locality().region(),
              xds_client_->bootstrap().node()->locality_region)
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.node().locality().zone(),
              xds_client_->bootstrap().node()->locality_zone)
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.node().locality().sub_zone(),
              xds_client_->bootstrap().node()->locality_sub_zone)
        << location.file() << ":" << location.line();
    if (xds_client_->bootstrap().node()->metadata.type() ==
        Json::Type::JSON_NULL) {
      EXPECT_FALSE(request.node().has_metadata())
          << location.file() << ":" << location.line();
    } else {
      std::string metadata_json_str;
      auto status =
          MessageToJsonString(request.node().metadata(), &metadata_json_str,
                              google::protobuf::util::JsonPrintOptions());
      ASSERT_TRUE(status.ok())
          << status << " on " << location.file() << ":" << location.line();
      auto metadata_json = Json::Parse(metadata_json_str);
      ASSERT_TRUE(metadata_json.ok())
          << metadata_json.status() << " on " << location.file() << ":"
          << location.line();
      EXPECT_EQ(*metadata_json, xds_client_->bootstrap().node()->metadata)
          << location.file() << ":" << location.line()
          << ":\nexpected: " << xds_client_->bootstrap().node()->metadata.Dump()
          << "\nactual: " << metadata_json->Dump();
    }
    // These are hard-coded by XdsClient.
    EXPECT_EQ(request.node().user_agent_name(),
              absl::StrCat("gRPC C-core ", GPR_PLATFORM_STRING))
        << location.file() << ":" << location.line();
    EXPECT_EQ(request.node().user_agent_version(),
              absl::StrCat("C-core ", grpc_version_string()))
        << location.file() << ":" << location.line();
    if (check_build_version) {
      auto build_version = GetBuildVersion(request.node());
      ASSERT_TRUE(build_version.has_value())
          << location.file() << ":" << location.line();
      EXPECT_EQ(*build_version,
                absl::StrCat("gRPC C-core ", GPR_PLATFORM_STRING, " ",
                             grpc_version_string()))
          << location.file() << ":" << location.line();
    }
  }

  // Helper function to find the "build_version" field, which was
  // removed in v3, but which we still populate in v2.
  static absl::optional<std::string> GetBuildVersion(
      const envoy::config::core::v3::Node& node) {
    const auto& unknown_field_set =
        node.GetReflection()->GetUnknownFields(node);
    for (int i = 0; i < unknown_field_set.field_count(); ++i) {
      const auto& unknown_field = unknown_field_set.field(i);
      if (unknown_field.number() == 5) return unknown_field.length_delimited();
    }
    return absl::nullopt;
  }

  RefCountedPtr<FakeXdsTransportFactory> transport_factory_;
  RefCountedPtr<XdsClient> xds_client_;
};

TEST_F(XdsClientTest, BasicWatch) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"A",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, UpdateFromServer) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Server sends an updated version of the resource.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("2")
          .set_nonce("B")
          .AddResource(XdsFooResource{"foo1", 9})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 9);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"2", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"2", /*response_nonce=*/"B",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, MultipleWatchersForSameResource) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Start a second watcher for the same resource.
  auto watcher2 = StartFooWatch("foo1");
  // This watcher should get an immediate notification, because the
  // resource is already cached.
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // Server should not have seen another request from the client.
  ASSERT_FALSE(stream->HaveMessageFromClient());
  // Server sends an updated version of the resource.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("2")
          .set_nonce("B")
          .AddResource(XdsFooResource{"foo1", 9})
          .Serialize());
  // XdsClient should deliver the response to both watchers.
  resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 9);
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 9);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"2", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel one of the watchers.
  CancelFooWatch(watcher.get(), "foo1");
  // The server should not see any new request.
  ASSERT_FALSE(WaitForRequest(stream.get()));
  // Now cancel the second watcher.
  CancelFooWatch(watcher2.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"2", /*response_nonce=*/"B",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, SubscribeToMultipleResources) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Start a watch for "foo2".
  auto watcher2 = StartFooWatch("foo2");
  // XdsClient should have sent a subscription request on the ADS stream.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("B")
          .AddResource(XdsFooResource{"foo2", 7})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo2");
  EXPECT_EQ(resource->value, 7);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Cancel watch for "foo1".
  CancelFooWatch(watcher.get(), "foo1");
  // XdsClient should send an unsubscription request.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(), /*resource_names=*/{"foo2"});
  // Now cancel watch for "foo2".
  CancelFooWatch(watcher2.get(), "foo2");
  // The XdsClient may or may not send another unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"B",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, UpdateContainsOnlyChangedResource) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Start a watch for "foo2".
  auto watcher2 = StartFooWatch("foo2");
  // XdsClient should have sent a subscription request on the ADS stream.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("B")
          .AddResource(XdsFooResource{"foo2", 7})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo2");
  EXPECT_EQ(resource->value, 7);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Server sends an update for "foo1".  The response does not contain "foo2".
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("2")
          .set_nonce("C")
          .AddResource(XdsFooResource{"foo1", 9})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 9);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"2", /*response_nonce=*/"C",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Cancel watch for "foo1".
  CancelFooWatch(watcher.get(), "foo1");
  // XdsClient should send an unsubscription request.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"2", /*response_nonce=*/"C",
               /*error_detail=*/absl::OkStatus(), /*resource_names=*/{"foo2"});
  // Now cancel watch for "foo2".
  CancelFooWatch(watcher2.get(), "foo2");
  // The XdsClient may or may not send another unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"2", /*response_nonce=*/"C",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, ResourceValidationFailure) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response containing an invalid resource.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddInvalidResource(XdsFooResourceType::Get()->type_url(),
                              "{\"name\":\"foo1\",\"value\":[]}")
          .Serialize());
  // XdsClient should deliver an error to the watcher.
  auto error = watcher->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "invalid resource: INVALID_ARGUMENT: errors validating JSON: "
            "[field:value error:is not a number] (node ID:xds_client_test)")
      << *error;
  // XdsClient should NACK the update.
  // Note that version_info is not populated in the request.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(
      *request, XdsFooResourceType::Get()->type_url(),
      /*version_info=*/"", /*response_nonce=*/"A",
      /*error_detail=*/
      absl::InvalidArgumentError(
          "xDS response validation errors: [resource index 0: foo1: "
          "validation error: INVALID_ARGUMENT: errors validating JSON: "
          "[field:value error:is not a number]]"),
      /*resource_names=*/{"foo1"});
  // Start a second watch for the same resource.  It should immediately
  // receive the same error.
  auto watcher2 = StartFooWatch("foo1");
  error = watcher2->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "invalid resource: INVALID_ARGUMENT: errors validating JSON: "
            "[field:value error:is not a number] (node ID:xds_client_test)")
      << *error;
  // Now server sends an updated version of the resource.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("2")
          .set_nonce("B")
          .AddResource(XdsFooResource{"foo1", 9})
          .Serialize());
  // XdsClient should deliver the response to both watchers.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 9);
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 9);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"2", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  CancelFooWatch(watcher2.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"2", /*response_nonce=*/"B",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, ResourceValidationFailureMultipleResources) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Before the server responds, add a watch for another resource.
  auto watcher2 = StartFooWatch("foo2");
  // Client should send another request.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Add a watch for a third resource.
  auto watcher3 = StartFooWatch("foo3");
  // Client should send another request.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2", "foo3"});
  // Add a watch for a fourth resource.
  auto watcher4 = StartFooWatch("foo4");
  // Client should send another request.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2", "foo3", "foo4"});
  // Server sends a response containing three invalid resources and one
  // valid resource.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          // foo1: JSON parsing succeeds, so we know the resource name,
          // but validation fails.
          .AddInvalidResource(XdsFooResourceType::Get()->type_url(),
                              "{\"name\":\"foo1\",\"value\":[]}")
          // foo2: JSON parsing fails, and not wrapped in a Resource
          // wrapper, so we don't actually know the resource's name.
          .AddInvalidResource(XdsFooResourceType::Get()->type_url(),
                              "{\"name\":\"foo2,\"value\":6}")
          // foo3: JSON parsing fails, but it is wrapped in a Resource
          // wrapper, so we do know the resource's name.
          .AddInvalidResource(XdsFooResourceType::Get()->type_url(),
                              "{\"name\":\"foo3,\"value\":6}",
                              /*resource_wrapper_name=*/"foo3")
          // foo4: valid resource.
          .AddResource(XdsFooResource{"foo4", 5})
          .Serialize());
  // XdsClient should deliver an error to the watchers for foo1 and foo3.
  auto error = watcher->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "invalid resource: INVALID_ARGUMENT: errors validating JSON: "
            "[field:value error:is not a number] (node ID:xds_client_test)")
      << *error;
  error = watcher3->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "invalid resource: INVALID_ARGUMENT: JSON parsing failed: "
            "[JSON parse error at index 15] (node ID:xds_client_test)")
      << *error;
  // It cannot delivery an error for foo2, because the client doesn't know
  // that that resource in the response was actually supposed to be foo2.
  EXPECT_FALSE(watcher2->HasError());
  // It will delivery a valid resource update for foo4.
  auto resource = watcher4->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo4");
  EXPECT_EQ(resource->value, 5);
  // XdsClient should NACK the update.
  // There was one good resource, so the version will be updated.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/
               absl::InvalidArgumentError(
                   "xDS response validation errors: ["
                   // foo1
                   "resource index 0: foo1: validation error: "
                   "INVALID_ARGUMENT: errors validating JSON: "
                   "[field:value error:is not a number]; "
                   // foo2 (name not known)
                   "resource index 1: INVALID_ARGUMENT: JSON parsing failed: "
                   "[JSON parse error at index 15]; "
                   // foo3
                   "resource index 2: foo3: validation error: "
                   "INVALID_ARGUMENT: JSON parsing failed: "
                   "[JSON parse error at index 15]]"),
               /*resource_names=*/{"foo1", "foo2", "foo3", "foo4"});
  // Cancel watches.
  CancelFooWatch(watcher.get(), "foo1", /*delay_unsubscription=*/true);
  CancelFooWatch(watcher.get(), "foo2", /*delay_unsubscription=*/true);
  CancelFooWatch(watcher.get(), "foo3", /*delay_unsubscription=*/true);
  CancelFooWatch(watcher.get(), "foo4");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"A",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, ResourceValidationFailureForCachedResource) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Send an update containing an invalid resource.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("2")
          .set_nonce("B")
          .AddInvalidResource(XdsFooResourceType::Get()->type_url(),
                              "{\"name\":\"foo1\",\"value\":[]}")
          .Serialize());
  // XdsClient should deliver an error to the watcher.
  auto error = watcher->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "invalid resource: INVALID_ARGUMENT: errors validating JSON: "
            "[field:value error:is not a number] (node ID:xds_client_test)")
      << *error;
  // XdsClient should NACK the update.
  // Note that version_info is set to the previous version in this request,
  // because there were no valid resources in it.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(
      *request, XdsFooResourceType::Get()->type_url(),
      /*version_info=*/"1", /*response_nonce=*/"B",
      /*error_detail=*/
      absl::InvalidArgumentError(
          "xDS response validation errors: [resource index 0: foo1: "
          "validation error: INVALID_ARGUMENT: errors validating JSON: "
          "[field:value error:is not a number]]"),
      /*resource_names=*/{"foo1"});
  // Start a second watcher for the same resource.  Even though the last
  // update was a NACK, we should still deliver the cached resource to
  // the watcher.
  // TODO(roth): Consider what the right behavior is here.  It seems
  // inconsistent that the watcher sees the error if it had started
  // before the error was seen but does not if it was started afterwards.
  // One option is to not send errors at all for already-cached resources;
  // another option is to send the errors even for newly started watchers.
  auto watcher2 = StartFooWatch("foo1");
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // Cancel watches.
  CancelFooWatch(watcher.get(), "foo1", /*delay_unsubscription=*/true);
  CancelFooWatch(watcher2.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"B",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, ResourceDoesNotExist) {
  InitXdsClient(FakeXdsBootstrap::Builder(), Duration::Seconds(1));
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Do not send a response, but wait for the resource to be reported as
  // not existing.
  EXPECT_TRUE(watcher->WaitForDoesNotExist(absl::Seconds(5)));
  // Start a new watcher for the same resource.  It should immediately
  // receive the same does-not-exist notification.
  auto watcher2 = StartFooWatch("foo1");
  EXPECT_TRUE(watcher2->WaitForDoesNotExist(absl::Seconds(1)));
  // Now server sends a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watchers.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  CancelFooWatch(watcher2.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"A",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, StreamClosedByServer) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Server sends a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Now server closes the stream.
  stream->MaybeSendStatusToClient(absl::OkStatus());
  // XdsClient should report error to watcher.
  auto error = watcher->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "xDS channel for server default_xds_server: xDS call failed; "
            "status: OK (node ID:xds_client_test)")
      << *error;
  // XdsClient should create a new stream.
  stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient sends a subscription request.
  // Note that the version persists from the previous stream, but the
  // nonce does not.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Before the server resends the resource, start a new watcher for the
  // same resource.  This watcher should immediately receive the cached
  // resource and then the error notification -- in that order.
  auto watcher2 = StartFooWatch("foo1");
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  error = watcher2->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "xDS channel for server default_xds_server: xDS call failed; "
            "status: OK (node ID:xds_client_test)")
      << *error;
  // Create a watcher for a new resource.  This should immediately
  // receive the cached channel error.
  auto watcher3 = StartFooWatch("foo2");
  error = watcher3->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "xDS channel for server default_xds_server: xDS call failed; "
            "status: OK (node ID:xds_client_test)")
      << *error;
  // And the client will send a new request subscribing to the new resource.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Server now sends the requested resources.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("B")
          .AddResource(XdsFooResource{"foo1", 6})
          .AddResource(XdsFooResource{"foo2", 7})
          .Serialize());
  // Watchers for foo1 do NOT get an update, since the resource has not changed.
  EXPECT_FALSE(watcher->WaitForNextResource());
  EXPECT_FALSE(watcher2->WaitForNextResource());
  // The watcher for foo2 gets the newly delivered resource.
  resource = watcher3->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo2");
  EXPECT_EQ(resource->value, 7);
  // XdsClient sends an ACK.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"B",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1", "foo2"});
  // Cancel watchers.
  CancelFooWatch(watcher.get(), "foo1", /*delay_unsubscription=*/true);
  CancelFooWatch(watcher2.get(), "foo1", /*delay_unsubscription=*/true);
  CancelFooWatch(watcher3.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"B",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, StreamClosedByServerAndResourcesNotResentOnNewStream) {
  // Lower resources-does-not-exist timeout, to make sure that we're not
  // triggering that here.
  InitXdsClient(FakeXdsBootstrap::Builder(), Duration::Seconds(3));
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Server sends a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Now server closes the stream.
  stream->MaybeSendStatusToClient(absl::OkStatus());
  // XdsClient should report error to watcher.
  auto error = watcher->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "xDS channel for server default_xds_server: xDS call failed; "
            "status: OK (node ID:xds_client_test)")
      << *error;
  // XdsClient should create a new stream.
  stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient sends a subscription request.
  // Note that the version persists from the previous stream, but the
  // nonce does not.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Server does NOT send the resource again.
  // Watcher should not get any update, since the resource has not changed.
  EXPECT_FALSE(watcher->WaitForNextResource());
  EXPECT_FALSE(watcher->HasError());
  EXPECT_FALSE(watcher->WaitForDoesNotExist(absl::Seconds(5)));
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, ConnectionFails) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Transport reports connection failure.
  transport_factory_->TriggerConnectionFailure(
      xds_client_->bootstrap().server(),
      absl::UnavailableError("connection failed"));
  // XdsClient should report an error to the watcher.
  auto error = watcher->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "xDS channel for server default_xds_server: "
            "connection failed (node ID:xds_client_test)")
      << *error;
  // Start a new watch.  This watcher should be given the same error,
  // since we have not yet recovered.
  auto watcher2 = StartFooWatch("foo1");
  error = watcher2->WaitForNextError();
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error->code(), absl::StatusCode::kUnavailable);
  EXPECT_EQ(error->message(),
            "xDS channel for server default_xds_server: "
            "connection failed (node ID:xds_client_test)")
      << *error;
  // Inside the XdsTransport interface, the channel will eventually
  // reconnect, and the call will proceed.  None of that will be visible
  // to the XdsClient, because the call uses wait_for_ready.  So here,
  // to simulate the connection being established, all we need to do is
  // allow the stream to proceed.
  // Server sends a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watchers.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  resource = watcher2->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"A",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, ResourceWrappedInResourceMessage) {
  InitXdsClient();
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = WaitForAdsStream();
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  CheckRequestNode(*request);  // Should be present on the first request.
  // Send a response with the resource wrapped in a Resource message.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6}, /*in_resource_wrapper=*/true)
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"A",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

TEST_F(XdsClientTest, BasicWatchV2) {
  InitXdsClient(FakeXdsBootstrap::Builder().set_use_v2());
  // Start a watch for "foo1".
  auto watcher = StartFooWatch("foo1");
  // Watcher should initially not see any resource reported.
  EXPECT_FALSE(watcher->WaitForNextResource().has_value());
  // XdsClient should have created an ADS stream.
  auto stream = transport_factory_->WaitForStream(
      xds_client_->bootstrap().server(), FakeXdsTransportFactory::kAdsV2Method,
      absl::Seconds(5) * grpc_test_slowdown_factor());
  ASSERT_TRUE(stream != nullptr);
  // XdsClient should have sent a subscription request on the ADS stream.
  auto request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->v2_type_url(),
               /*version_info=*/"", /*response_nonce=*/"",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Node Should be present on the first request.
  CheckRequestNode(*request, /*check_build_version=*/true);
  // Send a response.
  stream->SendMessageToClient(
      ResponseBuilder(XdsFooResourceType::Get()->v2_type_url())
          .set_version_info("1")
          .set_nonce("A")
          .AddResource(XdsFooResource{"foo1", 6})
          .Serialize());
  // XdsClient should have delivered the response to the watcher.
  auto resource = watcher->WaitForNextResource();
  ASSERT_TRUE(resource.has_value());
  EXPECT_EQ(resource->name, "foo1");
  EXPECT_EQ(resource->value, 6);
  // XdsClient should have sent an ACK message to the xDS server.
  request = WaitForRequest(stream.get());
  ASSERT_TRUE(request.has_value());
  CheckRequest(*request, XdsFooResourceType::Get()->v2_type_url(),
               /*version_info=*/"1", /*response_nonce=*/"A",
               /*error_detail=*/absl::OkStatus(),
               /*resource_names=*/{"foo1"});
  // Cancel watch.
  CancelFooWatch(watcher.get(), "foo1");
  // The XdsClient may or may not send an unsubscription message
  // before it closes the transport, depending on callback timing.
  request = WaitForRequest(stream.get());
  if (request.has_value()) {
    CheckRequest(*request, XdsFooResourceType::Get()->v2_type_url(),
                 /*version_info=*/"1", /*response_nonce=*/"A",
                 /*error_detail=*/absl::OkStatus(), /*resource_names=*/{});
  }
}

// FIXME: tests to add:
// - various federation tests from e2e tests
// - test that a channel failure only sends errors to watchers for the
//   channel that failed

}  // namespace
}  // namespace testing
}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  grpc::testing::TestEnvironment env(&argc, argv);
  grpc_init();
  int ret = RUN_ALL_TESTS();
  grpc_shutdown();
  return ret;
}

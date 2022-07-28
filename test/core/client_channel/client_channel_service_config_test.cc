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

#include "src/core/ext/filters/client_channel/client_channel_service_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "absl/strings/str_cat.h"

#include <grpc/grpc.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/gpr/string.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/service_config/service_config_impl.h"
#include "src/core/lib/service_config/service_config_parser.h"
#include "test/core/util/port.h"
#include "test/core/util/test_config.h"

namespace grpc_core {
namespace testing {

// A regular expression to enter referenced or child errors.
#define CHILD_ERROR_TAG ".*children.*"

class ClientChannelParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    builder_ = std::make_unique<CoreConfiguration::WithSubstituteBuilder>(
        [](CoreConfiguration::Builder* builder) {
          builder->service_config_parser()->RegisterParser(
              absl::make_unique<internal::ClientChannelServiceConfigParser>());
        });
    EXPECT_EQ(CoreConfiguration::Get().service_config_parser().GetParserIndex(
                  "client_channel"),
              0);
  }

 private:
  std::unique_ptr<CoreConfiguration::WithSubstituteBuilder> builder_;
};

TEST_F(ClientChannelParserTest, ValidLoadBalancingConfigPickFirst) {
  const char* test_json = "{\"loadBalancingConfig\": [{\"pick_first\":{}}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* parsed_config =
      static_cast<internal::ClientChannelGlobalParsedConfig*>(
          (*service_config)->GetGlobalParsedConfig(0));
  auto lb_config = parsed_config->parsed_lb_config();
  EXPECT_EQ(lb_config->name(), "pick_first");
}

TEST_F(ClientChannelParserTest, ValidLoadBalancingConfigRoundRobin) {
  const char* test_json =
      "{\"loadBalancingConfig\": [{\"round_robin\":{}}, {}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  auto parsed_config = static_cast<internal::ClientChannelGlobalParsedConfig*>(
      (*service_config)->GetGlobalParsedConfig(0));
  auto lb_config = parsed_config->parsed_lb_config();
  EXPECT_EQ(lb_config->name(), "round_robin");
}

TEST_F(ClientChannelParserTest, ValidLoadBalancingConfigGrpclb) {
  const char* test_json =
      "{\"loadBalancingConfig\": "
      "[{\"grpclb\":{\"childPolicy\":[{\"pick_first\":{}}]}}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* parsed_config =
      static_cast<internal::ClientChannelGlobalParsedConfig*>(
          (*service_config)->GetGlobalParsedConfig(0));
  auto lb_config = parsed_config->parsed_lb_config();
  EXPECT_EQ(lb_config->name(), "grpclb");
}

TEST_F(ClientChannelParserTest, ValidLoadBalancingConfigXds) {
  const char* test_json =
      "{\n"
      "  \"loadBalancingConfig\":[\n"
      "    { \"does_not_exist\":{} },\n"
      "    { \"xds_cluster_resolver_experimental\":{\n"
      "      \"discoveryMechanisms\": [\n"
      "      { \"clusterName\": \"foo\",\n"
      "        \"type\": \"EDS\"\n"
      "    } ]\n"
      "    } }\n"
      "  ]\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* parsed_config =
      static_cast<internal::ClientChannelGlobalParsedConfig*>(
          (*service_config)->GetGlobalParsedConfig(0));
  auto lb_config = parsed_config->parsed_lb_config();
  EXPECT_EQ(lb_config->name(), "xds_cluster_resolver_experimental");
}

TEST_F(ClientChannelParserTest, UnknownLoadBalancingConfig) {
  const char* test_json = "{\"loadBalancingConfig\": [{\"unknown\":{}}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: [errors validating JSON: ["
            "field:loadBalancingConfig error:"
            "No known policies in list: unknown]]");
}

TEST_F(ClientChannelParserTest, InvalidGrpclbLoadBalancingConfig) {
  const char* test_json =
      "{\"loadBalancingConfig\": ["
      "  {\"grpclb\":{\"childPolicy\":1}},"
      "  {\"round_robin\":{}}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      service_config.status().message(),
      "Service config parsing errors: [errors validating JSON: ["
      "field:loadBalancingConfig error:"
      "errors parsing grpclb LB policy config: ["
      "error parsing childPolicy field: type should be array]]]");
}

TEST_F(ClientChannelParserTest, ValidLoadBalancingPolicy) {
  const char* test_json = "{\"loadBalancingPolicy\":\"pick_first\"}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* parsed_config =
      static_cast<internal::ClientChannelGlobalParsedConfig*>(
          (*service_config)->GetGlobalParsedConfig(0));
  EXPECT_EQ(parsed_config->parsed_deprecated_lb_policy(), "pick_first");
}

TEST_F(ClientChannelParserTest, ValidLoadBalancingPolicyAllCaps) {
  const char* test_json = "{\"loadBalancingPolicy\":\"PICK_FIRST\"}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* parsed_config =
      static_cast<internal::ClientChannelGlobalParsedConfig*>(
          (*service_config)->GetGlobalParsedConfig(0));
  EXPECT_EQ(parsed_config->parsed_deprecated_lb_policy(), "pick_first");
}

TEST_F(ClientChannelParserTest, UnknownLoadBalancingPolicy) {
  const char* test_json = "{\"loadBalancingPolicy\":\"unknown\"}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      service_config.status().message(),
      "Service config parsing errors: [errors validating JSON: ["
      "field:loadBalancingPolicy error:unknown LB policy \"unknown\"]]");
}

TEST_F(ClientChannelParserTest, LoadBalancingPolicyXdsNotAllowed) {
  const char* test_json =
      "{\"loadBalancingPolicy\":\"xds_cluster_resolver_experimental\"}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      service_config.status().message(),
      "Service config parsing errors: [errors validating JSON: ["
      "field:loadBalancingPolicy error:LB policy "
      "\"xds_cluster_resolver_experimental\" requires a config. Please "
      "use loadBalancingConfig instead.]]");
}

TEST_F(ClientChannelParserTest, ValidTimeout) {
  const char* test_json =
      "{\n"
      "  \"methodConfig\": [ {\n"
      "    \"name\": [\n"
      "      { \"service\": \"TestServ\", \"method\": \"TestMethod\" }\n"
      "    ],\n"
      "    \"timeout\": \"5s\"\n"
      "  } ]\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* vector_ptr =
      (*service_config)
          ->GetMethodParsedConfigVector(
              grpc_slice_from_static_string("/TestServ/TestMethod"));
  ASSERT_NE(vector_ptr, nullptr);
  auto parsed_config = ((*vector_ptr)[0]).get();
  EXPECT_EQ(
      (static_cast<internal::ClientChannelMethodParsedConfig*>(parsed_config))
          ->timeout(),
      Duration::Seconds(5));
}

TEST_F(ClientChannelParserTest, InvalidTimeout) {
  const char* test_json =
      "{\n"
      "  \"methodConfig\": [ {\n"
      "    \"name\": [\n"
      "      { \"service\": \"service\", \"method\": \"method\" }\n"
      "    ],\n"
      "    \"timeout\": \"5sec\"\n"
      "  } ]\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      service_config.status().message(),
      "Service config parsing errors: [errors parsing methodConfig: ["
      "index 0: [errors validating JSON: ["
      "field:timeout error:Not a duration (no s suffix)]]]]");
}

TEST_F(ClientChannelParserTest, ValidWaitForReady) {
  const char* test_json =
      "{\n"
      "  \"methodConfig\": [ {\n"
      "    \"name\": [\n"
      "      { \"service\": \"TestServ\", \"method\": \"TestMethod\" }\n"
      "    ],\n"
      "    \"waitForReady\": true\n"
      "  } ]\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* vector_ptr =
      (*service_config)
          ->GetMethodParsedConfigVector(
              grpc_slice_from_static_string("/TestServ/TestMethod"));
  ASSERT_NE(vector_ptr, nullptr);
  auto parsed_config = ((*vector_ptr)[0]).get();
  ASSERT_TRUE(
      (static_cast<internal::ClientChannelMethodParsedConfig*>(parsed_config))
          ->wait_for_ready()
          .has_value());
  EXPECT_TRUE(
      (static_cast<internal::ClientChannelMethodParsedConfig*>(parsed_config))
          ->wait_for_ready()
          .value());
}

TEST_F(ClientChannelParserTest, InvalidWaitForReady) {
  const char* test_json =
      "{\n"
      "  \"methodConfig\": [ {\n"
      "    \"name\": [\n"
      "      { \"service\": \"service\", \"method\": \"method\" }\n"
      "    ],\n"
      "    \"waitForReady\": \"true\"\n"
      "  } ]\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      service_config.status().message(),
      "Service config parsing errors: [errors parsing methodConfig: ["
      "index 0: [errors validating JSON: ["
      "field:waitForReady error:is not a boolean]]]]");
}

TEST_F(ClientChannelParserTest, ValidHealthCheck) {
  const char* test_json =
      "{\n"
      "  \"healthCheckConfig\": {\n"
      "    \"serviceName\": \"health_check_service_name\"\n"
      "    }\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* parsed_config =
      static_cast<internal::ClientChannelGlobalParsedConfig*>(
          (*service_config)->GetGlobalParsedConfig(0));
  ASSERT_NE(parsed_config, nullptr);
  EXPECT_EQ(parsed_config->health_check_service_name(),
            "health_check_service_name");
}

TEST_F(ClientChannelParserTest, InvalidHealthCheckMultipleEntries) {
  const char* test_json =
      "{\n"
      "  \"healthCheckConfig\": {\n"
      "    \"serviceName\": \"health_check_service_name\"\n"
      "    },\n"
      "  \"healthCheckConfig\": {\n"
      "    \"serviceName\": \"health_check_service_name1\"\n"
      "    }\n"
      "}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "JSON parsing failed: ["
            "duplicate key \"healthCheckConfig\" at index 104]");
}

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

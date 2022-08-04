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

#include "src/core/lib/service_config/service_config.h"

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

// Set this channel arg to true to disable parsing.
#define GRPC_ARG_DISABLE_PARSING "disable_parsing"

class TestParsedConfig1 : public ServiceConfigParser::ParsedConfig {
 public:
  explicit TestParsedConfig1(int value) : value_(value) {}

  int value() const { return value_; }

 private:
  int value_;
};

class TestParser1 : public ServiceConfigParser::Parser {
 public:
  absl::string_view name() const override { return "test_parser_1"; }

  absl::StatusOr<std::unique_ptr<ServiceConfigParser::ParsedConfig>>
  ParseGlobalParams(const ChannelArgs& args, const Json& json) override {
    if (args.GetBool(GRPC_ARG_DISABLE_PARSING).value_or(false)) {
      return nullptr;
    }
    auto it = json.object_value().find("global_param");
    if (it != json.object_value().end()) {
      if (it->second.type() != Json::Type::NUMBER) {
        return absl::InvalidArgumentError(InvalidTypeErrorMessage());
      }
      int value = gpr_parse_nonnegative_int(it->second.string_value().c_str());
      if (value == -1) {
        return absl::InvalidArgumentError(InvalidValueErrorMessage());
      }
      return absl::make_unique<TestParsedConfig1>(value);
    }
    return nullptr;
  }

  static const char* InvalidTypeErrorMessage() {
    return "global_param value type should be a number";
  }

  static const char* InvalidValueErrorMessage() {
    return "global_param value type should be non-negative";
  }
};

class TestParser2 : public ServiceConfigParser::Parser {
 public:
  absl::string_view name() const override { return "test_parser_2"; }

  absl::StatusOr<std::unique_ptr<ServiceConfigParser::ParsedConfig>>
  ParsePerMethodParams(const ChannelArgs& args, const Json& json) override {
    if (args.GetBool(GRPC_ARG_DISABLE_PARSING).value_or(false)) {
      return nullptr;
    }
    auto it = json.object_value().find("method_param");
    if (it != json.object_value().end()) {
      if (it->second.type() != Json::Type::NUMBER) {
        return absl::InvalidArgumentError(InvalidTypeErrorMessage());
      }
      int value = gpr_parse_nonnegative_int(it->second.string_value().c_str());
      if (value == -1) {
        return absl::InvalidArgumentError(InvalidValueErrorMessage());
      }
      return absl::make_unique<TestParsedConfig1>(value);
    }
    return nullptr;
  }

  static const char* InvalidTypeErrorMessage() {
    return "method_param value type should be a number";
  }

  static const char* InvalidValueErrorMessage() {
    return "method_param value type should be non-negative";
  }
};

// This parser always adds errors
class ErrorParser : public ServiceConfigParser::Parser {
 public:
  explicit ErrorParser(absl::string_view name) : name_(name) {}

  absl::string_view name() const override { return name_; }

  absl::StatusOr<std::unique_ptr<ServiceConfigParser::ParsedConfig>>
  ParsePerMethodParams(const ChannelArgs& /*arg*/,
                       const Json& /*json*/) override {
    return absl::InvalidArgumentError(MethodError());
  }

  absl::StatusOr<std::unique_ptr<ServiceConfigParser::ParsedConfig>>
  ParseGlobalParams(const ChannelArgs& /*arg*/, const Json& /*json*/) override {
    return absl::InvalidArgumentError(GlobalError());
  }

  static const char* MethodError() { return "ErrorParser : methodError"; }

  static const char* GlobalError() { return "ErrorParser : globalError"; }

 private:
  absl::string_view name_;
};

class ServiceConfigTest : public ::testing::Test {
 protected:
  void SetUp() override {
    builder_ = std::make_unique<CoreConfiguration::WithSubstituteBuilder>(
        [](CoreConfiguration::Builder* builder) {
          builder->service_config_parser()->RegisterParser(
              absl::make_unique<TestParser1>());
          builder->service_config_parser()->RegisterParser(
              absl::make_unique<TestParser2>());
        });
    EXPECT_EQ(CoreConfiguration::Get().service_config_parser().GetParserIndex(
                  "test_parser_1"),
              0);
    EXPECT_EQ(CoreConfiguration::Get().service_config_parser().GetParserIndex(
                  "test_parser_2"),
              1);
  }

 private:
  std::unique_ptr<CoreConfiguration::WithSubstituteBuilder> builder_;
};

TEST_F(ServiceConfigTest, ErrorCheck1) {
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), "");
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(service_config.status().message()),
              ::testing::ContainsRegex("JSON parse error"));
}

TEST_F(ServiceConfigTest, BasicTest1) {
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), "{}");
  ASSERT_TRUE(service_config.ok()) << service_config.status();
}

TEST_F(ServiceConfigTest, SkipMethodConfigWithNoNameOrEmptyName) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"method_param\":1},"
      "  {\"name\":[], \"method_param\":1},"
      "  {\"name\":[{\"service\":\"TestServ\"}], \"method_param\":2}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  auto vector_ptr =
      (*service_config)
          ->GetMethodParsedConfigVector(
              grpc_slice_from_static_string("/TestServ/TestMethod"));
  ASSERT_EQ(vector_ptr->size(), 2UL);
  auto parsed_config = ((*vector_ptr)[1]).get();
  EXPECT_EQ(static_cast<TestParsedConfig1*>(parsed_config)->value(), 2);
}

TEST_F(ServiceConfigTest, ErrorDuplicateMethodConfigNames) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"name\":[{\"service\":\"TestServ\"}]},"
      "  {\"name\":[{\"service\":\"TestServ\"}]}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: ["
            "errors parsing methodConfig: ["
            "index 1: ["
            "field:name error:multiple method configs with same name]]]");
}

TEST_F(ServiceConfigTest, ErrorDuplicateMethodConfigNamesWithNullMethod) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"name\":[{\"service\":\"TestServ\",\"method\":null}]},"
      "  {\"name\":[{\"service\":\"TestServ\"}]}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: ["
            "errors parsing methodConfig: ["
            "index 1: ["
            "field:name error:multiple method configs with same name]]]");
}

TEST_F(ServiceConfigTest, ErrorDuplicateMethodConfigNamesWithEmptyMethod) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"name\":[{\"service\":\"TestServ\",\"method\":\"\"}]},"
      "  {\"name\":[{\"service\":\"TestServ\"}]}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: ["
            "errors parsing methodConfig: ["
            "index 1: ["
            "field:name error:multiple method configs with same name]]]");
}

TEST_F(ServiceConfigTest, ErrorDuplicateDefaultMethodConfigs) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"name\":[{}]},"
      "  {\"name\":[{}]}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: ["
            "errors parsing methodConfig: ["
            "index 1: ["
            "field:name error:multiple default method configs]]]");
}

TEST_F(ServiceConfigTest, ErrorDuplicateDefaultMethodConfigsWithNullService) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"name\":[{\"service\":null}]},"
      "  {\"name\":[{}]}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: ["
            "errors parsing methodConfig: ["
            "index 1: ["
            "field:name error:multiple default method configs]]]");
}

TEST_F(ServiceConfigTest, ErrorDuplicateDefaultMethodConfigsWithEmptyService) {
  const char* test_json =
      "{\"methodConfig\": ["
      "  {\"name\":[{\"service\":\"\"}]},"
      "  {\"name\":[{}]}"
      "]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            "Service config parsing errors: ["
            "errors parsing methodConfig: ["
            "index 1: ["
            "field:name error:multiple default method configs]]]");
}

TEST_F(ServiceConfigTest, ValidMethodConfig) {
  const char* test_json =
      "{\"methodConfig\": [{\"name\":[{\"service\":\"TestServ\"}]}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
}

TEST_F(ServiceConfigTest, Parser1BasicTest1) {
  const char* test_json = "{\"global_param\":5}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  EXPECT_EQ((static_cast<TestParsedConfig1*>(
                 (*service_config)->GetGlobalParsedConfig(0)))
                ->value(),
            5);
  EXPECT_EQ((*service_config)
                ->GetMethodParsedConfigVector(
                    grpc_slice_from_static_string("/TestServ/TestMethod")),
            nullptr);
}

TEST_F(ServiceConfigTest, Parser1BasicTest2) {
  const char* test_json = "{\"global_param\":1000}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  EXPECT_EQ((static_cast<TestParsedConfig1*>(
                 (*service_config)->GetGlobalParsedConfig(0)))
                ->value(),
            1000);
}

TEST_F(ServiceConfigTest, Parser1DisabledViaChannelArg) {
  const ChannelArgs args = ChannelArgs().Set(GRPC_ARG_DISABLE_PARSING, 1);
  const char* test_json = "{\"global_param\":5}";
  auto service_config = ServiceConfigImpl::Create(args, test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  EXPECT_EQ((*service_config)->GetGlobalParsedConfig(0), nullptr);
}

TEST_F(ServiceConfigTest, Parser1ErrorInvalidType) {
  const char* test_json = "{\"global_param\":\"5\"}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            absl::StrCat("Service config parsing errors: [",
                         TestParser1::InvalidTypeErrorMessage(), "]"));
}

TEST_F(ServiceConfigTest, Parser1ErrorInvalidValue) {
  const char* test_json = "{\"global_param\":-5}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            absl::StrCat("Service config parsing errors: [",
                         TestParser1::InvalidValueErrorMessage(), "]"));
}

TEST_F(ServiceConfigTest, Parser2BasicTest) {
  const char* test_json =
      "{\"methodConfig\": [{\"name\":[{\"service\":\"TestServ\"}], "
      "\"method_param\":5}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* vector_ptr =
      (*service_config)
          ->GetMethodParsedConfigVector(
              grpc_slice_from_static_string("/TestServ/TestMethod"));
  ASSERT_NE(vector_ptr, nullptr);
  auto parsed_config = ((*vector_ptr)[1]).get();
  EXPECT_EQ(static_cast<TestParsedConfig1*>(parsed_config)->value(), 5);
}

TEST_F(ServiceConfigTest, Parser2DisabledViaChannelArg) {
  const ChannelArgs args = ChannelArgs().Set(GRPC_ARG_DISABLE_PARSING, 1);
  const char* test_json =
      "{\"methodConfig\": [{\"name\":[{\"service\":\"TestServ\"}], "
      "\"method_param\":5}]}";
  auto service_config = ServiceConfigImpl::Create(args, test_json);
  ASSERT_TRUE(service_config.ok()) << service_config.status();
  const auto* vector_ptr =
      (*service_config)
          ->GetMethodParsedConfigVector(
              grpc_slice_from_static_string("/TestServ/TestMethod"));
  ASSERT_NE(vector_ptr, nullptr);
  auto parsed_config = ((*vector_ptr)[1]).get();
  EXPECT_EQ(parsed_config, nullptr);
}

TEST_F(ServiceConfigTest, Parser2ErrorInvalidType) {
  const char* test_json =
      "{\"methodConfig\": [{\"name\":[{\"service\":\"TestServ\"}], "
      "\"method_param\":\"5\"}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            absl::StrCat("Service config parsing errors: ["
                         "errors parsing methodConfig: ["
                         "index 0: [",
                         TestParser2::InvalidTypeErrorMessage(), "]]]"));
}

TEST_F(ServiceConfigTest, Parser2ErrorInvalidValue) {
  const char* test_json =
      "{\"methodConfig\": [{\"name\":[{\"service\":\"TestServ\"}], "
      "\"method_param\":-5}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            absl::StrCat("Service config parsing errors: ["
                         "errors parsing methodConfig: ["
                         "index 0: [",
                         TestParser2::InvalidValueErrorMessage(), "]]]"));
}

TEST(ServiceConfigParserTest, DoubleRegistration) {
  CoreConfiguration::Reset();
  ASSERT_DEATH_IF_SUPPORTED(
      CoreConfiguration::WithSubstituteBuilder builder(
          [](CoreConfiguration::Builder* builder) {
            builder->service_config_parser()->RegisterParser(
                absl::make_unique<ErrorParser>("xyzabc"));
            builder->service_config_parser()->RegisterParser(
                absl::make_unique<ErrorParser>("xyzabc"));
          }),
      "xyzabc.*already registered");
}

// Test parsing with ErrorParsers which always add errors
class ErroredParsersScopingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    builder_ = std::make_unique<CoreConfiguration::WithSubstituteBuilder>(
        [](CoreConfiguration::Builder* builder) {
          builder->service_config_parser()->RegisterParser(
              absl::make_unique<ErrorParser>("ep1"));
          builder->service_config_parser()->RegisterParser(
              absl::make_unique<ErrorParser>("ep2"));
        });
    EXPECT_EQ(
        CoreConfiguration::Get().service_config_parser().GetParserIndex("ep1"),
        0);
    EXPECT_EQ(
        CoreConfiguration::Get().service_config_parser().GetParserIndex("ep2"),
        1);
  }

 private:
  std::unique_ptr<CoreConfiguration::WithSubstituteBuilder> builder_;
};

TEST_F(ErroredParsersScopingTest, GlobalParams) {
  const char* test_json = "{}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(service_config.status().message(),
            absl::StrCat("Service config parsing errors: [",
                         ErrorParser::GlobalError(), "; ",
                         ErrorParser::GlobalError(), "]"));
}

TEST_F(ErroredParsersScopingTest, MethodParams) {
  const char* test_json = "{\"methodConfig\": [{}]}";
  auto service_config = ServiceConfigImpl::Create(ChannelArgs(), test_json);
  EXPECT_EQ(service_config.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      service_config.status().message(),
      absl::StrCat("Service config parsing errors: [",
                   ErrorParser::GlobalError(), "; ", ErrorParser::GlobalError(),
                   "; "
                   "errors parsing methodConfig: ["
                   "index 0: [",
                   ErrorParser::MethodError(), "; ", ErrorParser::MethodError(),
                   "]]]"));
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

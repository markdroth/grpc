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

#include <grpc/support/port_platform.h>

#include "src/core/ext/filters/stateful_session_affinity/stateful_session_affinity_service_config_parser.h"

#include <vector>

#include "absl/types/optional.h"

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/channel/status_util.h"

namespace grpc_core {

const JsonLoaderInterface*
StatefulSessionAffinityMethodParsedConfig::CookieConfig::JsonLoader(
    const JsonArgs&) {
  static const auto* loader = JsonObjectLoader<CookieConfig>()
                                  .OptionalField("name", &CookieConfig::name)
                                  .OptionalField("path", &CookieConfig::path)
                                  .OptionalField("ttl", &CookieConfig::ttl)
                                  .Finish();
  return loader;
}

void StatefulSessionAffinityMethodParsedConfig::CookieConfig::JsonPostLoad(
    const Json&, const JsonArgs&, ValidationErrors* errors) {
  // Validate that cookie_name is non-empty.
  if (name.has_value() && name->empty()) {
    ValidationErrors::ScopedField field(errors, ".name");
    errors->AddError("must be non-empty");
  }
}

const JsonLoaderInterface*
StatefulSessionAffinityMethodParsedConfig::JsonLoader(const JsonArgs&) {
  static const auto* loader =
      JsonObjectLoader<StatefulSessionAffinityMethodParsedConfig>()
          .OptionalField("statefulSessionAffinity",
                         &StatefulSessionAffinityMethodParsedConfig::configs_)
          .Finish();
  return loader;
}

std::unique_ptr<ServiceConfigParser::ParsedConfig>
StatefulSessionAffinityServiceConfigParser::ParsePerMethodParams(
    const ChannelArgs& args, const Json& json, ValidationErrors* errors) {
  // Only parse config if the following channel arg is present.
  if (!args.GetBool(GRPC_ARG_PARSE_STATEFUL_SESSION_AFFINITY_METHOD_CONFIG)
           .value_or(false)) {
    return nullptr;
  }
  // Parse config from json.
  return LoadFromJson<
      std::unique_ptr<StatefulSessionAffinityMethodParsedConfig>>(
      json, JsonArgs(), errors);
}

void StatefulSessionAffinityServiceConfigParser::Register(
    CoreConfiguration::Builder* builder) {
  builder->service_config_parser()->RegisterParser(
      std::make_unique<StatefulSessionAffinityServiceConfigParser>());
}

size_t StatefulSessionAffinityServiceConfigParser::ParserIndex() {
  return CoreConfiguration::Get().service_config_parser().GetParserIndex(
      parser_name());
}

}  // namespace grpc_core

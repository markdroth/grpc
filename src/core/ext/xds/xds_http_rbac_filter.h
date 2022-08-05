//
// Copyright 2021 gRPC authors.
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

#ifndef GRPC_CORE_EXT_XDS_XDS_HTTP_RBAC_FILTER_H
#define GRPC_CORE_EXT_XDS_XDS_HTTP_RBAC_FILTER_H

#include <grpc/support/port_platform.h>

#include "absl/status/statusor.h"
#include "upb/arena.h"
#include "upb/def.h"
#include "upb/upb.h"

#include "src/core/ext/xds/xds_http_filters.h"
#include "src/core/ext/xds/xds_http_filters_grpc.h"
#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/channel/channel_fwd.h"

namespace grpc_core {

class XdsHttpRbacFilter : public GrpcXdsHttpFilter {
 public:
  absl::string_view ConfigProtoType() const override;
  absl::string_view OverrideConfigProtoType() const override;
  void PopulateSymtab(upb_DefPool* symtab) const override;
  absl::StatusOr<FilterConfig> GenerateFilterConfig(
      upb_StringView serialized_filter_config, upb_Arena* arena) const override;
  absl::StatusOr<FilterConfig> GenerateFilterConfigOverride(
      upb_StringView serialized_filter_config, upb_Arena* arena) const override;
  const grpc_channel_filter* channel_filter() const override;
  ChannelArgs ModifyChannelArgs(const ChannelArgs& args) const override;
  absl::StatusOr<ServiceConfigJsonEntry> GenerateServiceConfig(
      const FilterConfig& hcm_filter_config,
      const FilterConfig* filter_config_override) const override;
  bool IsSupportedOnClients() const override { return false; }
  bool IsSupportedOnServers() const override { return true; }
};

}  // namespace grpc_core

#endif  // GRPC_CORE_EXT_XDS_XDS_HTTP_RBAC_FILTER_H

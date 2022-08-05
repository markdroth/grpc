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

#ifndef GRPC_CORE_EXT_XDS_XDS_CLIENT_GRPC_H
#define GRPC_CORE_EXT_XDS_XDS_CLIENT_GRPC_H

#include <grpc/support/port_platform.h>

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

#include <grpc/impl/codegen/grpc_types.h>

#include "src/core/ext/xds/certificate_provider_store.h"
#include "src/core/ext/xds/xds_bootstrap.h"
#include "src/core/ext/xds/xds_client.h"
#include "src/core/ext/xds/xds_http_filters.h"
#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/gprpp/orphanable.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/iomgr_fwd.h"
#include "src/core/lib/json/json.h"

namespace grpc_core {

class GrpcXdsClient : public XdsClient {
 public:
  // Factory function to get or create the global XdsClient instance.
  static absl::StatusOr<RefCountedPtr<GrpcXdsClient>> GetOrCreate(
      const ChannelArgs& args, const char* reason);

  // Do not instantiate directly -- use GetOrCreate() instead.
  GrpcXdsClient(
      std::unique_ptr<XdsBootstrap> bootstrap,
      XdsHttpFilterRegistry xds_http_filter_registry,
      XdsClusterSpecifierPluginRegistry xds_cluster_specifier_plugin_registry,
      XdsLbPolicyRegistry xds_lb_policy_registry, const ChannelArgs& args);
  ~GrpcXdsClient() override;

  // Helpers for encoding the XdsClient object in channel args.
  static absl::string_view ChannelArgName() {
    return "grpc.internal.xds_client";
  }
  static int ChannelArgsCompare(const XdsClient* a, const XdsClient* b) {
    return QsortCompare(a, b);
  }

  grpc_pollset_set* interested_parties() const;

  CertificateProviderStore& certificate_provider_store() const {
    return *certificate_provider_store_;
  }

 private:
  OrphanablePtr<CertificateProviderStore> certificate_provider_store_;
};

namespace internal {
void SetXdsChannelArgsForTest(grpc_channel_args* args);
void UnsetGlobalXdsClientForTest();
// Sets bootstrap config to be used when no env var is set.
// Does not take ownership of config.
void SetXdsFallbackBootstrapConfig(const char* config);
}  // namespace internal

// Exposed for testing purposes only.
class GrpcXdsCertificateProviderPluginMap
    : public XdsCertificateProviderPluginMapInterface {
 public:
  const CertificateProviderStore::PluginDefinitionMap& plugin_map() const {
    return plugin_map_;
  }

  absl::Status AddPlugin(const std::string& instance_name,
                         const std::string& plugin_name,
                         const Json& config) override;

  bool HasPlugin(const std::string& instance_name) const override;

  std::string ToString() const override;

 private:
  CertificateProviderStore::PluginDefinitionMap plugin_map_;
};

}  // namespace grpc_core

#endif  // GRPC_CORE_EXT_XDS_XDS_CLIENT_GRPC_H

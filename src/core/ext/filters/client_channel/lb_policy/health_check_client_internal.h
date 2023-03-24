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

#ifndef GRPC_SRC_CORE_EXT_FILTERS_CLIENT_CHANNEL_LB_POLICY_HEALTH_CHECK_CLIENT_INTERNAL_H
#define GRPC_SRC_CORE_EXT_FILTERS_CLIENT_CHANNEL_LB_POLICY_HEALTH_CHECK_CLIENT_INTERNAL_H

#include <grpc/support/port_platform.h>

#include <memory>

#include "google/protobuf/duration.upb.h"
#include "upb/upb.hpp"
#include "xds/data/orca/v3/orca_load_report.upb.h"
#include "xds/service/orca/v3/orca.upb.h"

#include <grpc/status.h>

#include "src/core/ext/filters/client_channel/subchannel.h"
#include "src/core/ext/filters/client_channel/subchannel_interface_internal.h"
#include "src/core/ext/filters/client_channel/subchannel_stream_client.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/gprpp/unique_type_name.h"
#include "src/core/lib/load_balancing/lb_policy.h"
#include "src/core/lib/load_balancing/subchannel_interface.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/transport/error_utils.h"

namespace grpc_core {

class HealthWatcher;

// This producer is registered with a subchannel.  It creates a streaming
// health watch call for each health check service name that is being
// watched and reports the resulting connectivity state to all
// registered watchers.
class HealthProducer : public Subchannel::DataProducerInterface {
 public:
  void Start(RefCountedPtr<Subchannel> subchannel);

  void Orphan() override;

  static UniqueTypeName Type() {
    static UniqueTypeName::Factory kFactory("health_check");
    return kFactory.Create();
  }

  UniqueTypeName type() const override { return Type(); }

  void AddWatcher(HealthWatcher* watcher,
                  const std::string& health_check_service_name);
  void RemoveWatcher(HealthWatcher* watcher,
                     const std::string& health_check_service_name);

 private:
  class ConnectivityWatcher;
  class HealthChecker;

  // Handles a connectivity state change on the subchannel.
  void OnConnectivityStateChange(grpc_connectivity_state state);

  RefCountedPtr<Subchannel> subchannel_;
  RefCountedPtr<ConnectedSubchannel> connected_subchannel_;
  ConnectivityWatcher* connectivity_watcher_;

  Mutex mu_;
  std::map<std::string /*health_check_service_name*/,
           OrphanablePtr<HealthChecker>>
      health_checkers_ ABSL_GUARDED_BY(&mu_);
};

// A data watcher that handles health checking.
class HealthWatcher : public InternalSubchannelDataWatcherInterface {
 public:
  HealthWatcher(
      std::shared_ptr<WorkSerializer> work_serializer,
      absl::string_view health_check_service_name,
      std::unique_ptr<SubchannelInterface::ConnectivityStateWatcherInterface>
          watcher)
      : work_serializer_(std::move(work_serializer)),
        health_check_service_name_(health_check_service_name),
        watcher_(std::move(watcher)) {}
  ~HealthWatcher() override;

  // When the client channel sees this wrapper, it will pass it the real
  // subchannel to use.
  void SetSubchannel(Subchannel* subchannel) override;

  void Notify(grpc_connectivity_state state, absl::Status status);

 private:
  std::shared_ptr<WorkSerializer> work_serializer_;
  std::string health_check_service_name_;
  std::shared_ptr<SubchannelInterface::ConnectivityStateWatcherInterface>
      watcher_;
  RefCountedPtr<HealthProducer> producer_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_FILTERS_CLIENT_CHANNEL_LB_POLICY_HEALTH_CHECK_CLIENT_INTERNAL_H

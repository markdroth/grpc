// Copyright 2023 gRPC authors.
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

// Auto generated by tools/codegen/core/gen_experiments.py
//
// This file contains the autogenerated parts of the experiments API.
//
// It generates two symbols for each experiment.
//
// For the experiment named new_car_project, it generates:
//
// - a function IsNewCarProjectEnabled() that returns true if the experiment
//   should be enabled at runtime.
//
// - a macro GRPC_EXPERIMENT_IS_INCLUDED_NEW_CAR_PROJECT that is defined if the
//   experiment *could* be enabled at runtime.
//
// The function is used to determine whether to run the experiment or
// non-experiment code path.
//
// If the experiment brings significant bloat, the macro can be used to avoid
// including the experiment code path in the binary for binaries that are size
// sensitive.
//
// By default that includes our iOS and Android builds.
//
// Finally, a small array is included that contains the metadata for each
// experiment.
//
// A macro, GRPC_EXPERIMENTS_ARE_FINAL, controls whether we fix experiment
// configuration at build time (if it's defined) or allow it to be tuned at
// runtime (if it's disabled).
//
// If you are using the Bazel build system, that macro can be configured with
// --define=grpc_experiments_are_final=true

#ifndef GRPC_SRC_CORE_LIB_EXPERIMENTS_EXPERIMENTS_H
#define GRPC_SRC_CORE_LIB_EXPERIMENTS_EXPERIMENTS_H

#include <grpc/support/port_platform.h>

#include <stddef.h>

#include "src/core/lib/experiments/config.h"

namespace grpc_core {

#ifdef GRPC_EXPERIMENTS_ARE_FINAL

#if defined(GRPC_CFSTREAM)
inline bool IsTcpFrameSizeTuningEnabled() { return false; }
inline bool IsTcpRcvLowatEnabled() { return false; }
inline bool IsPeerStateBasedFramingEnabled() { return false; }
inline bool IsMemoryPressureControllerEnabled() { return false; }
inline bool IsUnconstrainedMaxQuotaBufferSizeEnabled() { return false; }
inline bool IsEventEngineClientEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_MONITORING_EXPERIMENT
inline bool IsMonitoringExperimentEnabled() { return true; }
inline bool IsPromiseBasedClientCallEnabled() { return false; }
inline bool IsFreeLargeAllocatorEnabled() { return false; }
inline bool IsPromiseBasedServerCallEnabled() { return false; }
inline bool IsEventEngineListenerEnabled() { return false; }
inline bool IsScheduleCancellationOverWriteEnabled() { return false; }
inline bool IsTraceRecordCallopsEnabled() { return false; }
inline bool IsEventEngineDnsEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_WORK_STEALING
inline bool IsWorkStealingEnabled() { return true; }
inline bool IsClientPrivacyEnabled() { return false; }
inline bool IsCanaryClientPrivacyEnabled() { return false; }
inline bool IsServerPrivacyEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_UNIQUE_METADATA_STRINGS
inline bool IsUniqueMetadataStringsEnabled() { return true; }
inline bool IsKeepaliveFixEnabled() { return false; }
inline bool IsKeepaliveServerFixEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_LAZIER_STREAM_UPDATES
inline bool IsLazierStreamUpdatesEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_JITTER_MAX_IDLE
inline bool IsJitterMaxIdleEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_ROUND_ROBIN_DELEGATE_TO_PICK_FIRST
inline bool IsRoundRobinDelegateToPickFirstEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_WRR_DELEGATE_TO_PICK_FIRST
inline bool IsWrrDelegateToPickFirstEnabled() { return true; }

#elif defined(GPR_WINDOWS)
inline bool IsTcpFrameSizeTuningEnabled() { return false; }
inline bool IsTcpRcvLowatEnabled() { return false; }
inline bool IsPeerStateBasedFramingEnabled() { return false; }
inline bool IsMemoryPressureControllerEnabled() { return false; }
inline bool IsUnconstrainedMaxQuotaBufferSizeEnabled() { return false; }
inline bool IsEventEngineClientEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_MONITORING_EXPERIMENT
inline bool IsMonitoringExperimentEnabled() { return true; }
inline bool IsPromiseBasedClientCallEnabled() { return false; }
inline bool IsFreeLargeAllocatorEnabled() { return false; }
inline bool IsPromiseBasedServerCallEnabled() { return false; }
inline bool IsEventEngineListenerEnabled() { return false; }
inline bool IsScheduleCancellationOverWriteEnabled() { return false; }
inline bool IsTraceRecordCallopsEnabled() { return false; }
inline bool IsEventEngineDnsEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_WORK_STEALING
inline bool IsWorkStealingEnabled() { return true; }
inline bool IsClientPrivacyEnabled() { return false; }
inline bool IsCanaryClientPrivacyEnabled() { return false; }
inline bool IsServerPrivacyEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_UNIQUE_METADATA_STRINGS
inline bool IsUniqueMetadataStringsEnabled() { return true; }
inline bool IsKeepaliveFixEnabled() { return false; }
inline bool IsKeepaliveServerFixEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_LAZIER_STREAM_UPDATES
inline bool IsLazierStreamUpdatesEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_JITTER_MAX_IDLE
inline bool IsJitterMaxIdleEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_ROUND_ROBIN_DELEGATE_TO_PICK_FIRST
inline bool IsRoundRobinDelegateToPickFirstEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_WRR_DELEGATE_TO_PICK_FIRST
inline bool IsWrrDelegateToPickFirstEnabled() { return true; }

#else
inline bool IsTcpFrameSizeTuningEnabled() { return false; }
inline bool IsTcpRcvLowatEnabled() { return false; }
inline bool IsPeerStateBasedFramingEnabled() { return false; }
inline bool IsMemoryPressureControllerEnabled() { return false; }
inline bool IsUnconstrainedMaxQuotaBufferSizeEnabled() { return false; }
inline bool IsEventEngineClientEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_MONITORING_EXPERIMENT
inline bool IsMonitoringExperimentEnabled() { return true; }
inline bool IsPromiseBasedClientCallEnabled() { return false; }
inline bool IsFreeLargeAllocatorEnabled() { return false; }
inline bool IsPromiseBasedServerCallEnabled() { return false; }
inline bool IsEventEngineListenerEnabled() { return false; }
inline bool IsScheduleCancellationOverWriteEnabled() { return false; }
inline bool IsTraceRecordCallopsEnabled() { return false; }
inline bool IsEventEngineDnsEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_WORK_STEALING
inline bool IsWorkStealingEnabled() { return true; }
inline bool IsClientPrivacyEnabled() { return false; }
inline bool IsCanaryClientPrivacyEnabled() { return false; }
inline bool IsServerPrivacyEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_UNIQUE_METADATA_STRINGS
inline bool IsUniqueMetadataStringsEnabled() { return true; }
inline bool IsKeepaliveFixEnabled() { return false; }
inline bool IsKeepaliveServerFixEnabled() { return false; }
#define GRPC_EXPERIMENT_IS_INCLUDED_LAZIER_STREAM_UPDATES
inline bool IsLazierStreamUpdatesEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_JITTER_MAX_IDLE
inline bool IsJitterMaxIdleEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_ROUND_ROBIN_DELEGATE_TO_PICK_FIRST
inline bool IsRoundRobinDelegateToPickFirstEnabled() { return true; }
#define GRPC_EXPERIMENT_IS_INCLUDED_WRR_DELEGATE_TO_PICK_FIRST
inline bool IsWrrDelegateToPickFirstEnabled() { return true; }
#endif

#else
#define GRPC_EXPERIMENT_IS_INCLUDED_TCP_FRAME_SIZE_TUNING
inline bool IsTcpFrameSizeTuningEnabled() { return IsExperimentEnabled(0); }
#define GRPC_EXPERIMENT_IS_INCLUDED_TCP_RCV_LOWAT
inline bool IsTcpRcvLowatEnabled() { return IsExperimentEnabled(1); }
#define GRPC_EXPERIMENT_IS_INCLUDED_PEER_STATE_BASED_FRAMING
inline bool IsPeerStateBasedFramingEnabled() { return IsExperimentEnabled(2); }
#define GRPC_EXPERIMENT_IS_INCLUDED_MEMORY_PRESSURE_CONTROLLER
inline bool IsMemoryPressureControllerEnabled() {
  return IsExperimentEnabled(3);
}
#define GRPC_EXPERIMENT_IS_INCLUDED_UNCONSTRAINED_MAX_QUOTA_BUFFER_SIZE
inline bool IsUnconstrainedMaxQuotaBufferSizeEnabled() {
  return IsExperimentEnabled(4);
}
#define GRPC_EXPERIMENT_IS_INCLUDED_EVENT_ENGINE_CLIENT
inline bool IsEventEngineClientEnabled() { return IsExperimentEnabled(5); }
#define GRPC_EXPERIMENT_IS_INCLUDED_MONITORING_EXPERIMENT
inline bool IsMonitoringExperimentEnabled() { return IsExperimentEnabled(6); }
#define GRPC_EXPERIMENT_IS_INCLUDED_PROMISE_BASED_CLIENT_CALL
inline bool IsPromiseBasedClientCallEnabled() { return IsExperimentEnabled(7); }
#define GRPC_EXPERIMENT_IS_INCLUDED_FREE_LARGE_ALLOCATOR
inline bool IsFreeLargeAllocatorEnabled() { return IsExperimentEnabled(8); }
#define GRPC_EXPERIMENT_IS_INCLUDED_PROMISE_BASED_SERVER_CALL
inline bool IsPromiseBasedServerCallEnabled() { return IsExperimentEnabled(9); }
#define GRPC_EXPERIMENT_IS_INCLUDED_EVENT_ENGINE_LISTENER
inline bool IsEventEngineListenerEnabled() { return IsExperimentEnabled(10); }
#define GRPC_EXPERIMENT_IS_INCLUDED_SCHEDULE_CANCELLATION_OVER_WRITE
inline bool IsScheduleCancellationOverWriteEnabled() {
  return IsExperimentEnabled(11);
}
#define GRPC_EXPERIMENT_IS_INCLUDED_TRACE_RECORD_CALLOPS
inline bool IsTraceRecordCallopsEnabled() { return IsExperimentEnabled(12); }
#define GRPC_EXPERIMENT_IS_INCLUDED_EVENT_ENGINE_DNS
inline bool IsEventEngineDnsEnabled() { return IsExperimentEnabled(13); }
#define GRPC_EXPERIMENT_IS_INCLUDED_WORK_STEALING
inline bool IsWorkStealingEnabled() { return IsExperimentEnabled(14); }
#define GRPC_EXPERIMENT_IS_INCLUDED_CLIENT_PRIVACY
inline bool IsClientPrivacyEnabled() { return IsExperimentEnabled(15); }
#define GRPC_EXPERIMENT_IS_INCLUDED_CANARY_CLIENT_PRIVACY
inline bool IsCanaryClientPrivacyEnabled() { return IsExperimentEnabled(16); }
#define GRPC_EXPERIMENT_IS_INCLUDED_SERVER_PRIVACY
inline bool IsServerPrivacyEnabled() { return IsExperimentEnabled(17); }
#define GRPC_EXPERIMENT_IS_INCLUDED_UNIQUE_METADATA_STRINGS
inline bool IsUniqueMetadataStringsEnabled() { return IsExperimentEnabled(18); }
#define GRPC_EXPERIMENT_IS_INCLUDED_KEEPALIVE_FIX
inline bool IsKeepaliveFixEnabled() { return IsExperimentEnabled(19); }
#define GRPC_EXPERIMENT_IS_INCLUDED_KEEPALIVE_SERVER_FIX
inline bool IsKeepaliveServerFixEnabled() { return IsExperimentEnabled(20); }
#define GRPC_EXPERIMENT_IS_INCLUDED_LAZIER_STREAM_UPDATES
inline bool IsLazierStreamUpdatesEnabled() { return IsExperimentEnabled(21); }
#define GRPC_EXPERIMENT_IS_INCLUDED_JITTER_MAX_IDLE
inline bool IsJitterMaxIdleEnabled() { return IsExperimentEnabled(22); }
#define GRPC_EXPERIMENT_IS_INCLUDED_ROUND_ROBIN_DELEGATE_TO_PICK_FIRST
inline bool IsRoundRobinDelegateToPickFirstEnabled() {
  return IsExperimentEnabled(23);
}
#define GRPC_EXPERIMENT_IS_INCLUDED_WRR_DELEGATE_TO_PICK_FIRST
inline bool IsWrrDelegateToPickFirstEnabled() {
  return IsExperimentEnabled(24);
}

constexpr const size_t kNumExperiments = 25;
extern const ExperimentMetadata g_experiment_metadata[kNumExperiments];

#endif
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_EXPERIMENTS_EXPERIMENTS_H

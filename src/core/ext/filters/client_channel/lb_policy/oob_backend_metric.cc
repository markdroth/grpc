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

#include "src/core/ext/filters/client_channel/lb_policy/oob_backend_metric.h"

#include "google/protobuf/duration.upb.h"
#include "upb/upb.hpp"
#include "xds/data/orca/v3/orca_load_report.upb.h"
#include "xds/service/orca/v3/orca.upb.h"

#include <grpc/status.h>

#include "src/core/ext/filters/client_channel/backend_metric.h"
#include "src/core/ext/filters/client_channel/subchannel.h"
#include "src/core/ext/filters/client_channel/subchannel_interface_internal.h"
#include "src/core/ext/filters/client_channel/subchannel_stream_client.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/transport/error_utils.h"

namespace grpc_core {

namespace {

TraceFlag grpc_orca_client_trace(false, "orca_client");

constexpr char kProducerType[] = "orca";

class OrcaWatcher;

// This producer is registered with a subchannel.  It creates a
// streaming ORCA call and reports the resulting backend metrics to all
// registered watchers.
class OrcaProducer : public Subchannel::DataProducerInterface {
 public:
  explicit OrcaProducer(RefCountedPtr<Subchannel> subchannel);

  void Orphan() override;

  const char* type() override { return kProducerType; }

  // Adds and removes watchers.
  void AddWatcher(WeakRefCountedPtr<OrcaWatcher> watcher);
  void RemoveWatcher(OrcaWatcher* watcher);

 private:
  class ConnectivityWatcher;
  class OrcaStreamEventHandler;

  // Returns the minimum requested reporting interval across all watchers.
  Duration GetMinIntervalLocked() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(&mu_);

  // Starts a new stream.
  // Called whenever the reporting interval changes or the subchannel
  // transitions to state READY.
  void StartStreamLocked() ABSL_EXCLUSIVE_LOCKS_REQUIRED(&mu_);

  // Handles a connectivity state change on the subchannel.
  void OnConnectivityStateChange(grpc_connectivity_state state);

  // Called to notify watchers of a new backend metric report.
  void NotifyWatchers(
      const LoadBalancingPolicy::BackendMetricAccessor::BackendMetricData&
          backend_metric_data);

  RefCountedPtr<Subchannel> subchannel_;
  RefCountedPtr<ConnectedSubchannel> connected_subchannel_;
  ConnectivityWatcher* connectivity_watcher_;
  Mutex mu_;
  // TODO(roth): Use std::set<> instead once we can use C++14 heterogenous
  // map lookups.
  std::map<OrcaWatcher*, WeakRefCountedPtr<OrcaWatcher>> watcher_map_
      ABSL_GUARDED_BY(mu_);
  Duration report_interval_ ABSL_GUARDED_BY(mu_) = Duration::Infinity();
  OrphanablePtr<SubchannelStreamClient> stream_client_ ABSL_GUARDED_BY(mu_);
};

// This watcher is returned to the LB policy and added to the
// client channel SubchannelWrapper.
class OrcaWatcher : public SubchannelInterface::DataWatcherInterface {
 public:
  OrcaWatcher(Duration report_interval,
              std::unique_ptr<OobBackendMetricWatcher> watcher)
      : report_interval_(report_interval), watcher_(std::move(watcher)) {}

  void Orphan() override;

  Duration report_interval() const { return report_interval_; }

  // When the client channel sees this wrapper, it will pass it the real
  // subchannel to use.
  void SetSubchannel(Subchannel* subchannel) override;

  OobBackendMetricWatcher* watcher() const { return watcher_.get(); }

 private:
  Duration report_interval_;
  std::unique_ptr<OobBackendMetricWatcher> watcher_;
  RefCountedPtr<OrcaProducer> producer_;
};

//
// OrcaProducer::ConnectivityWatcher
//

class OrcaProducer::ConnectivityWatcher
    : public Subchannel::ConnectivityStateWatcherInterface {
 public:
  explicit ConnectivityWatcher(WeakRefCountedPtr<OrcaProducer> producer)
      : producer_(std::move(producer)),
        interested_parties_(grpc_pollset_set_create()) {}

  ~ConnectivityWatcher() override {
    grpc_pollset_set_destroy(interested_parties_);
  }

  void OnConnectivityStateChange() override {
    auto change = PopConnectivityStateChange();
    producer_->OnConnectivityStateChange(change.state);
  }

  grpc_pollset_set* interested_parties() override {
    return interested_parties_;
  }

 private:
  WeakRefCountedPtr<OrcaProducer> producer_;
  grpc_pollset_set* interested_parties_;
};

//
// OrcaProducer::OrcaStreamEventHandler
//

class OrcaProducer::OrcaStreamEventHandler
    : public SubchannelStreamClient::CallEventHandler {
 public:
  explicit OrcaStreamEventHandler(WeakRefCountedPtr<OrcaProducer> producer)
      : producer_(std::move(producer)) {}

  Slice GetPathLocked() override {
    return Slice::FromStaticString(
        "/xds.service.orca.v3.OpenRcaService/StreamCoreMetrics");
  }

  void OnCallStartLocked(SubchannelStreamClient* client) override {}

  void OnRetryTimerStartLocked(SubchannelStreamClient* client) override {}

  grpc_slice EncodeSendMessageLocked() override {
    upb::Arena arena;
    xds_service_orca_v3_OrcaLoadReportRequest* request =
        xds_service_orca_v3_OrcaLoadReportRequest_new(arena.ptr());
    gpr_timespec timespec = producer_->report_interval_.as_timespec();
    auto* report_interval =
        xds_service_orca_v3_OrcaLoadReportRequest_mutable_report_interval(
            request, arena.ptr());
    google_protobuf_Duration_set_seconds(report_interval, timespec.tv_sec);
    google_protobuf_Duration_set_nanos(report_interval, timespec.tv_nsec);
    size_t buf_length;
    char* buf = xds_service_orca_v3_OrcaLoadReportRequest_serialize(
        request, arena.ptr(), &buf_length);
    grpc_slice request_slice = GRPC_SLICE_MALLOC(buf_length);
    memcpy(GRPC_SLICE_START_PTR(request_slice), buf, buf_length);
    return request_slice;
  }

  absl::Status RecvMessageReadyLocked(
      SubchannelStreamClient* client,
      absl::string_view serialized_message) override {
    BackendMetricAllocator allocator;
    auto* backend_metric_data =
        ParseBackendMetricData(serialized_message, &allocator);
    if (backend_metric_data == nullptr) {
      return absl::InvalidArgumentError("unable to parse Orca response");
    }
    producer_->NotifyWatchers(*backend_metric_data);
    return absl::OkStatus();
  }

  void RecvTrailingMetadataReadyLocked(SubchannelStreamClient* client,
                                       grpc_status_code status) override {
    if (status == GRPC_STATUS_UNIMPLEMENTED) {
      gpr_log(GPR_ERROR, "Orca stream returned UNIMPLEMENTED; disabling");
    }
  }

 private:
  class BackendMetricAllocator : public BackendMetricAllocatorInterface {
   public:
    BackendMetricAllocator() = default;

    LoadBalancingPolicy::BackendMetricAccessor::BackendMetricData*
    AllocateBackendMetricData() override {
      return &backend_metric_data_;
    }

    char* AllocateString(size_t size) override {
      char* string = static_cast<char*>(gpr_malloc(size));
      string_storage_.emplace_back(string);
      return string;
    }

   private:
    LoadBalancingPolicy::BackendMetricAccessor::BackendMetricData
        backend_metric_data_;
    std::vector<UniquePtr<char>> string_storage_;
  };

  WeakRefCountedPtr<OrcaProducer> producer_;
};

//
// OrcaProducer
//

OrcaProducer::OrcaProducer(RefCountedPtr<Subchannel> subchannel)
    : subchannel_(std::move(subchannel)) {
  subchannel_->AddDataProducer(this);
  connected_subchannel_ = subchannel_->connected_subchannel();
  auto connectivity_watcher = MakeRefCounted<ConnectivityWatcher>(WeakRef());
  connectivity_watcher_ = connectivity_watcher.get();
  subchannel_->WatchConnectivityState(
      connected_subchannel_ == nullptr ? GRPC_CHANNEL_IDLE : GRPC_CHANNEL_READY,
      /*health_check_service_name=*/absl::nullopt,
      std::move(connectivity_watcher));
}

void OrcaProducer::Orphan() {
  subchannel_->CancelConnectivityStateWatch(
      /*health_check_service_name=*/absl::nullopt, connectivity_watcher_);
  subchannel_->RemoveDataProducer(this);
  stream_client_.reset();
}

void OrcaProducer::AddWatcher(WeakRefCountedPtr<OrcaWatcher> watcher) {
  MutexLock lock(&mu_);
  Duration watcher_interval = watcher->report_interval();
  watcher_map_[watcher.get()] = std::move(watcher);
  if (watcher_interval < report_interval_) {
    report_interval_ = watcher_interval;
    stream_client_.reset();
    StartStreamLocked();
  }
}

void OrcaProducer::RemoveWatcher(OrcaWatcher* watcher) {
  MutexLock lock(&mu_);
  watcher_map_.erase(watcher);
  if (watcher_map_.empty()) {
    stream_client_.reset();
    return;
  }
  Duration new_interval = GetMinIntervalLocked();
  if (new_interval < report_interval_) {
    report_interval_ = new_interval;
    stream_client_.reset();
    StartStreamLocked();
  }
}

Duration OrcaProducer::GetMinIntervalLocked() const {
  Duration duration = Duration::Infinity();
  for (const auto& p : watcher_map_) {
    Duration watcher_interval = p.first->report_interval();
    if (watcher_interval < duration) duration = watcher_interval;
  }
  return duration;
}

void OrcaProducer::StartStreamLocked() {
  stream_client_ = MakeOrphanable<SubchannelStreamClient>(
      connected_subchannel_, subchannel_->pollset_set(),
      absl::make_unique<OrcaStreamEventHandler>(WeakRef()),
      GRPC_TRACE_FLAG_ENABLED(grpc_orca_client_trace) ? "OrcaClient" : nullptr);
}

void OrcaProducer::NotifyWatchers(
    const LoadBalancingPolicy::BackendMetricAccessor::BackendMetricData&
        backend_metric_data) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_orca_client_trace)) {
    gpr_log(GPR_INFO, "OrcaProducer %p: reporting backend metrics to watchers",
            this);
  }
  MutexLock lock(&mu_);
  for (const auto& p : watcher_map_) {
    p.first->watcher()->OnBackendMetricReport(backend_metric_data);
  }
}

void OrcaProducer::OnConnectivityStateChange(grpc_connectivity_state state) {
  MutexLock lock(&mu_);
  if (state == GRPC_CHANNEL_READY) {
    connected_subchannel_ = subchannel_->connected_subchannel();
    if (!watcher_map_.empty()) StartStreamLocked();
  } else {
    connected_subchannel_.reset();
    stream_client_.reset();
  }
}

//
// OrcaWatcher
//

void OrcaWatcher::Orphan() {
  if (producer_ != nullptr) {
    producer_->RemoveWatcher(this);
    producer_.reset();
  }
}

void OrcaWatcher::SetSubchannel(Subchannel* subchannel) {
  // Check if our producer is already registered with the subchannel.
  // If not, create a new one, which will register itself with the subchannel.
  auto* p =
      static_cast<OrcaProducer*>(subchannel->GetDataProducer(kProducerType));
  if (p != nullptr) producer_ = p->RefIfNonZero();
  if (producer_ == nullptr) {
    producer_ = MakeRefCounted<OrcaProducer>(subchannel->Ref());
  }
  // Register ourself with the producer.
  producer_->AddWatcher(WeakRef());
}

}  // namespace

RefCountedPtr<SubchannelInterface::DataWatcherInterface>
MakeOobBackendMetricWatcher(Duration report_interval,
                            std::unique_ptr<OobBackendMetricWatcher> watcher) {
  return MakeRefCounted<OrcaWatcher>(report_interval, std::move(watcher));
}

}  // namespace grpc_core

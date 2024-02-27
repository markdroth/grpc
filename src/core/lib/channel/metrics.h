// Copyright 2024 The gRPC Authors.
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

#ifndef GRPC_SRC_CORE_LIB_CHANNEL_METRICS_H
#define GRPC_SRC_CORE_LIB_CHANNEL_METRICS_H

#include <grpc/support/port_platform.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/functional/function_ref.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"

#include <grpc/support/log.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gprpp/no_destruct.h"
#include "src/core/lib/gprpp/sync.h"

namespace grpc_core {

constexpr absl::string_view kMetricLabelTarget = "grpc.target";

// An interface for implementing callback-style uint64 metrics.
// To be implemented by stats plugins.
class AsyncUIn64MetricReporter {
 public:
  virtual ~AsyncUIn64MetricReporter;
  ;
  = default;

  virtual void Report(uint64_t value,
                      absl::Span<const absl::string_view> label_values,
                      absl::Span<const absl::string_view> optional_values) = 0;
};

// An interface for implementing callback-style double metrics.
// To be implemented by stats plugins.
class AsyncDoubleMetricReporter {
 public:
  virtual ~AsyncDoubleMetricReporter() = default;

  virtual void Report(double value,
                      absl::Span<const absl::string_view> label_values,
                      absl::Span<const absl::string_view> optional_values) = 0;
};

// A global registry of instruments(metrics). This API is designed to be used
// to register instruments (Counter, Histogram, and Gauge) as part of program
// startup, before the execution of the main function (during dynamic
// initialization time). Using this API after the main function begins may
// result into missing instruments. This API is thread-unsafe.
class GlobalInstrumentsRegistry {
 public:
  enum class ValueType {
    kUndefined,
    kUInt64,
    kDouble,
  };
  enum class InstrumentType {
    kUndefined,
    kCounter,
    kHistogram,
    kGauge,
  };
  using UInt64ReporterCallback =
      std::function<void(void*, AsyncUIn64MetricReporter&)>;
  using DoubleReporterCallback =
      std::function<void(void*, AsyncDoubleMetricReporter&)>;
  struct GlobalInstrumentDescriptor {
    ValueType value_type;
    InstrumentType instrument_type;
    uint32_t index;
    bool enable_by_default;
    absl::string_view name;
    absl::string_view description;
    absl::string_view unit;
    std::vector<absl::string_view> label_keys;
    std::vector<absl::string_view> optional_label_keys;
    absl::variant<UInt64ReporterCallback, DoubleReporterCallback> reporter;
  };
  struct GlobalHandle {
    // This is the index for the corresponding registered instrument that
    // StatsPlugins can use to uniquely identify an instrument in the current
    // process. Though this is not guaranteed to be stable between different
    // runs or between different versions.
    uint32_t index;
  };
  struct GlobalUInt64CounterHandle : public GlobalHandle {};
  struct GlobalDoubleCounterHandle : public GlobalHandle {};
  struct GlobalUInt64HistogramHandle : public GlobalHandle {};
  struct GlobalDoubleHistogramHandle : public GlobalHandle {};
  struct GlobalUInt64GaugeHandle : public GlobalHandle {};
  struct GlobalDoubleGaugeHandle : public GlobalHandle {};
  struct GlobalAsyncUInt64GaugeHandle : public GlobalHandle {};
  struct GlobalAsyncDoubleGaugeHandle : public GlobalHandle {};

  // Creates instrument in the GlobalInstrumentsRegistry.
  static GlobalUInt64CounterHandle RegisterUInt64Counter(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default);
  static GlobalDoubleCounterHandle RegisterDoubleCounter(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default);
  static GlobalUInt64HistogramHandle RegisterUInt64Histogram(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default);
  static GlobalDoubleHistogramHandle RegisterDoubleHistogram(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default);
  static GlobalUInt64GaugeHandle RegisterUInt64Gauge(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default);
  static GlobalDoubleGaugeHandle RegisterDoubleGauge(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default);
  static GlobalAsyncUInt64GaugeHandle RegisterAsyncUInt64Gauge(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default, UInt64ReporterCallback callback);
  static GGGGlobalAsyncDoubleGaugeHandlegisterAsyncDoubleGauge(
      absl::string_view name, absl::string_view description,
      absl::string_view unit, absl::Span<const absl::string_view> label_keys,
      absl::Span<const absl::string_view> optional_label_keys,
      bool enable_by_default, DoubleReporterCallback callback);

  static void ForEach(
      absl::FunctionRef<void(const GlobalInstrumentDescriptor&)> f);

 private:
  friend class GlobalInstrumentsRegistryTestPeer;

  GlobalInstrumentsRegistry() = delete;

  static absl::flat_hash_map<
      absl::string_view, GlobalInstrumentsRegistry::GlobalInstrumentDescriptor>&
  GetInstrumentList();
};

// The StatsPlugin interface.
class StatsPlugin {
 public:
  class ChannelScope {
   public:
    ChannelScope(absl::string_view target, absl::string_view authority)
        : target_(target), authority_(authority) {}

    absl::string_view target() const { return target_; }
    absl::string_view authority() const { return authority_; }

   private:
    absl::string_view target_;
    absl::string_view authority_;
  };

  virtual ~StatsPlugin() = default;

  virtual bool IsEnabledForChannel(const ChannelScope& scope) const = 0;
  virtual bool IsEnabledForServer(const ChannelArgs& args) const = 0;

  virtual void AddCounter(
      GlobalInstrumentsRegistry::GlobalUInt64CounterHandle handle,
      uint64_t value, absl::Span<const absl::string_view> label_values,
      absl::Span<const absl::string_view> optional_values) = 0;
  virtual void AddCounter(
      GlobalInstrumentsRegistry::GlobalDoubleCounterHandle handle, double value,
      absl::Span<const absl::string_view> label_values,
      absl::Span<const absl::string_view> optional_values) = 0;
  virtual void RecordHistogram(
      GlobalInstrumentsRegistry::GlobalUInt64HistogramHandle handle,
      uint64_t value, absl::Span<const absl::string_view> label_values,
      absl::Span<const absl::string_view> optional_values) = 0;
  virtual void RecordHistogram(
      GlobalInstrumentsRegistry::GlobalDoubleHistogramHandle handle,
      double value, absl::Span<const absl::string_view> label_values,
      absl::Span<const absl::string_view> optional_values) = 0;
  virtual void SetGauge(
      GlobalInstrumentsRegistry::GlobalUInt64GaugeHandle handle, uint64_t value,
      absl::Span<const absl::string_view> label_values,
      absl::Span<const absl::string_view> optional_values) = 0;
  virtual void SetGauge(
      GlobalInstrumentsRegistry::GlobalDoubleGaugeHandle handle, double value,
      absl::Span<const absl::string_view> label_values,
      absl::Span<const absl::string_view> optional_values) = 0;
  virtual void AddAsyncGaugeEntity(
      GlobalInstrumentsRegistry::GlobalAsyncUInt64GaugeHandle handle,
      void* entity) = 0;
  virtual void RemoveAsyncGaugeEntity(
      GlobalInstrumentsRegistry::GlobalAsyncUInt64GaugeHandle handle,
      void* entity) = 0;
  virtual void AddAsyncGaugeEntity(
      GlobalInstrumentsRegistry::GlobalAsyncDoubleGaugeHandle handle,
      void* entity) = 0;
  virtual void RemoveAsyncGaugeEntity(
      GlobalInstrumentsRegistry::GlobalAsyncDoubleGaugeHandle handle,
      void* entity) = 0;
  // TODO(yijiem): Details pending.
  // std::unique_ptr<AsyncInstrument> GetObservableGauge(
  //     absl::string_view name, absl::string_view description,
  //     absl::string_view unit);
  // AsyncInstrument* GetObservableCounter(
  //     absl::string_view name, absl::string_view description,
  //     absl::string_view unit);

  // TODO(yijiem): This is an optimization for the StatsPlugin to create its own
  // representation of the label_values and use it multiple times. We would
  // change AddCounter and RecordHistogram to take RefCountedPtr<LabelValueSet>
  // and also change the StatsPluginsGroup to support this.
  // Use the StatsPlugin to get a representation of label values that can be
  // saved for multiple uses later.
  // virtual RefCountedPtr<LabelValueSet> MakeLabelValueSet(
  //     absl::Span<absl::string_view> label_values) = 0;
};

// A global registry of StatsPlugins. It has shared ownership to the registered
// StatsPlugins. This API is supposed to be used during runtime after the main
// function begins. This API is thread-safe.
class GlobalStatsPluginRegistry {
 public:
  class StatsPluginGroup {
   public:
    void push_back(std::shared_ptr<StatsPlugin> plugin) {
      plugins_.push_back(std::move(plugin));
    }

    void AddCounter(GlobalInstrumentsRegistry::GlobalUInt64CounterHandle handle,
                    uint64_t value,
                    absl::Span<const absl::string_view> label_values,
                    absl::Span<const absl::string_view> optional_values) {
      for (auto& plugin : plugins_) {
        plugin->AddCounter(handle, value, label_values, optional_values);
      }
    }
    void AddCounter(GlobalInstrumentsRegistry::GlobalDoubleCounterHandle handle,
                    double value,
                    absl::Span<const absl::string_view> label_values,
                    absl::Span<const absl::string_view> optional_values) {
      for (auto& plugin : plugins_) {
        plugin->AddCounter(handle, value, label_values, optional_values);
      }
    }
    void RecordHistogram(
        GlobalInstrumentsRegistry::GlobalUInt64HistogramHandle handle,
        uint64_t value, absl::Span<const absl::string_view> label_values,
        absl::Span<const absl::string_view> optional_values) {
      for (auto& plugin : plugins_) {
        plugin->RecordHistogram(handle, value, label_values, optional_values);
      }
    }
    void RecordHistogram(
        GlobalInstrumentsRegistry::GlobalDoubleHistogramHandle handle,
        double value, absl::Span<const absl::string_view> label_values,
        absl::Span<const absl::string_view> optional_values) {
      for (auto& plugin : plugins_) {
        plugin->RecordHistogram(handle, value, label_values, optional_values);
      }
    }
    void SetGauge(GlobalInstrumentsRegistry::GlobalUInt64GaugeHandle handle,
                  uint64_t value,
                  absl::Span<const absl::string_view> label_values,
                  absl::Span<const absl::string_view> optional_values) {
      for (auto& plugin : plugins_) {
        plugin->SetGauge(handle, value, label_values, optional_values);
      }
    }
    void SetGauge(GlobalInstrumentsRegistry::GlobalDoubleGaugeHandle handle,
                  double value,
                  absl::Span<const absl::string_view> label_values,
                  absl::Span<const absl::string_view> optional_values) {
      for (auto& plugin : plugins_) {
        plugin->SetGauge(handle, value, label_values, optional_values);
      }
    }
    void AddAsyncGaugeEntity(
        GlobalInstrumentsRegistry::GlobalAsyncUInt64GaugeHandle handle,
        void* entity) {
      for (auto& plugin : plugins_) {
        plugin->AddAsyncGaugeEntity(handle, entity);
      }
    }
    void RemoveAsyncGaugeEntity(
        GlobalInstrumentsRegistry::GlobalAsyncUInt64GaugeHandle handle,
        void* entity) {
      for (auto& plugin : plugins_) {
        plugin->RemoveAsyncGaugeEntity(handle, entity);
      }
    }
    void AddAsyncGaugeEntity(
        GlobalInstrumentsRegistry::GlobalAsyncDoubleGaugeHandle handle,
        void* entity) {
      for (auto& plugin : plugins_) {
        plugin->AddAsyncGaugeEntity(handle, entity);
      }
    }
    void RemoveAsyncGaugeEntity(
        GlobalInstrumentsRegistry::GlobalAsyncDoubleGaugeHandle handle,
        void* entity) {
      for (auto& plugin : plugins_) {
        plugin->RemoveAsyncGaugeEntity(handle, entity);
      }
    }

   private:
    std::vector<std::shared_ptr<StatsPlugin>> plugins_;
  };

  static void RegisterStatsPlugin(std::shared_ptr<StatsPlugin> plugin);
  // The following two functions can be invoked to get a StatsPluginGroup for
  // a specified scope.
  static StatsPluginGroup GetStatsPluginsForChannel(
      const StatsPlugin::ChannelScope& scope);
  // TODO(yijiem): Implement this.
  // StatsPluginsGroup GetStatsPluginsForServer(ChannelArgs& args);

 private:
  friend class GlobalStatsPluginRegistryTestPeer;

  GlobalStatsPluginRegistry() = default;

  static NoDestruct<Mutex> mutex_;
  static NoDestruct<std::vector<std::shared_ptr<StatsPlugin>>> plugins_
      ABSL_GUARDED_BY(mutex_);
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_CHANNEL_METRICS_H

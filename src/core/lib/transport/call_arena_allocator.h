// Copyright 2024 gRPC authors.
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

#ifndef GRPC_SRC_CORE_LIB_TRANSPORT_CALL_ARENA_ALLOCATOR_H
#define GRPC_SRC_CORE_LIB_TRANSPORT_CALL_ARENA_ALLOCATOR_H

#include <stddef.h>

#include <atomic>
#include <cstddef>

#include <grpc/support/port_platform.h>

#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/memory_quota.h"
#include "src/core/util/ref_counted.h"

namespace grpc_core {

class CallSizeEstimator final {
 public:
  explicit CallSizeEstimator(size_t initial_estimate)
      : call_size_estimate_(initial_estimate) {}

  size_t CallSizeEstimate() {
    // We round up our current estimate to the NEXT value of kRoundUpSize.
    // This ensures:
    //  1. a consistent size allocation when our estimate is drifting slowly
    //     (which is common) - which tends to help most allocators reuse memory
    //  2. a small amount of allowed growth over the estimate without hitting
    //     the arena size doubling case, reducing overall memory usage
    static constexpr size_t kRoundUpSize = 256;
    return (call_size_estimate_.load(std::memory_order_relaxed) +
            2 * kRoundUpSize) &
           ~(kRoundUpSize - 1);
  }

  void UpdateCallSizeEstimate(size_t size);

 private:
  std::atomic<size_t> call_size_estimate_;
};

class CallArenaAllocator final : public ArenaFactory {
 public:
  CallArenaAllocator(MemoryAllocator allocator, size_t initial_size)
      : ArenaFactory(std::move(allocator)),
        call_size_estimator_(initial_size) {}

  RefCountedPtr<Arena> MakeArena() override {
    return Arena::Create(call_size_estimator_.CallSizeEstimate(), Ref());
  }

  void FinalizeArena(Arena* arena) override {
    call_size_estimator_.UpdateCallSizeEstimate(arena->TotalUsedBytes());
  }

 private:
  CallSizeEstimator call_size_estimator_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_TRANSPORT_CALL_ARENA_ALLOCATOR_H

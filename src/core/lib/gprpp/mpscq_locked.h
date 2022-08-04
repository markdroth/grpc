/*
 *
 * Copyright 2016 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_LIB_GPRPP_MPSCQ_LOCKED_H
#define GRPC_CORE_LIB_GPRPP_MPSCQ_LOCKED_H

#include <grpc/support/port_platform.h>

#include "src/core/lib/gprpp/mpscq.h"
#include "src/core/lib/gprpp/sync.h"

namespace grpc_core {

// An mpscq with a lock: it's safe to pop from multiple threads, but doing
// only one thread will succeed concurrently.
class LockedMultiProducerSingleConsumerQueue {
 public:
  typedef MultiProducerSingleConsumerQueue::Node Node;

  // Push a node
  // Thread safe - can be called from multiple threads concurrently
  // Returns true if this was possibly the first node (may return true
  // sporadically, will not return false sporadically)
  bool Push(Node* node);

  // Pop a node (returns NULL if no node is ready - which doesn't indicate that
  // the queue is empty!!)
  // Thread safe - can be called from multiple threads concurrently
  Node* TryPop();

  // Pop a node.  Returns NULL only if the queue was empty at some point after
  // calling this function
  Node* Pop();

 private:
  MultiProducerSingleConsumerQueue queue_;
  Mutex mu_;
};

}  // namespace grpc_core

#endif /* GRPC_CORE_LIB_GPRPP_MPSCQ_LOCKED_H */

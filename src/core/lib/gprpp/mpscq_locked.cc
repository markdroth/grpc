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

#include <grpc/support/port_platform.h>

#include "src/core/lib/gprpp/mpscq_locked.h"

namespace grpc_core {

bool LockedMultiProducerSingleConsumerQueue::Push(Node* node) {
  return queue_.Push(node);
}

LockedMultiProducerSingleConsumerQueue::Node*
LockedMultiProducerSingleConsumerQueue::TryPop() {
  if (mu_.TryLock()) {
    Node* node = queue_.Pop();
    mu_.Unlock();
    return node;
  }
  return nullptr;
}

LockedMultiProducerSingleConsumerQueue::Node*
LockedMultiProducerSingleConsumerQueue::Pop() {
  MutexLock lock(&mu_);
  bool empty = false;
  Node* node;
  do {
    node = queue_.PopAndCheckEnd(&empty);
  } while (node == nullptr && !empty);
  return node;
}

}  // namespace grpc_core

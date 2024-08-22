//
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
//

#ifndef GRPC_SRC_CORE_UTIL_LRU_CACHE_H
#define GRPC_SRC_CORE_UTIL_LRU_CACHE_H

#include <list>
#include <tuple>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/functional/any_invocable.h"
#include "absl/log/check.h"
#include "absl/types/optional.h"

namespace grpc_core {

// A simple LRU cache.  Retains at most max_size entries.
// Caller is responsible for synchronization.
// TODO(roth): Support heterogenous lookups.
template<typename Key, typename Value>
class LruCache {
 public:
  explicit LruCache(size_t max_size) : max_size_(max_size) {
    CHECK_GT(max_size, 0);
  }

  // Returns the value for key, or nullopt if not present.
  absl::optional<Value> Get(Key key);

  // If key is present in the cache, returns the corresponding value.
  // Otherwise, inserts a new entry in the map, calling create() to
  // construct the new value.  If inserting a new entry causes the cache
  // to be too large, removes the least recently used entry.
  Value GetOrInsert(Key key, absl::AnyInvocable<Value(Key)> create);

 private:
  struct CacheEntry {
    Value value;
    typename std::list<Key>::iterator lru_iterator;

    explicit CacheEntry(Value v) : value(std::move(v)) {}
  };

  const size_t max_size_;
  absl::flat_hash_map<Key, CacheEntry> cache_;
  std::list<Key> lru_list_;
};

//
// implementation -- no user-serviceable parts below
//

template<typename Key, typename Value>
absl::optional<Value> LruCache<Key, Value>::Get(Key key) {
  auto it = cache_.find(key);
  if (it == cache_.end()) return absl::nullopt;
  // Found the entry.  Move the entry to the end of the LRU list.
  auto new_lru_it =
      lru_list_.insert(lru_list_.end(), *it->second.lru_iterator);
  lru_list_.erase(it->second.lru_iterator);
  it->second.lru_iterator = new_lru_it;
  return it->second.value;
}

template<typename Key, typename Value>
Value LruCache<Key, Value>::GetOrInsert(
    Key key, absl::AnyInvocable<Value(Key)> create) {
  auto value = Get(key);
  if (value.has_value()) return std::move(*value);
  // Entry not found.  We'll need to insert a new entry.
  // If the cache is at max size, remove the least recently used entry.
  if (cache_.size() == max_size_) {
    auto lru_it = lru_list_.begin();
    CHECK(lru_it != lru_list_.end());
    auto cache_it = cache_.find(*lru_it);
    CHECK(cache_it != cache_.end());
    cache_.erase(cache_it);
    lru_list_.pop_front();
  }
  // Create a new entry, insert it, and return it.
  auto it = cache_.emplace(std::piecewise_construct,
                           std::forward_as_tuple(key),
                           std::forward_as_tuple(create(key)))
                .first;
  it->second.lru_iterator = lru_list_.insert(lru_list_.end(), key);
  return it->second.value;
}

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_UTIL_LRU_CACHE_H

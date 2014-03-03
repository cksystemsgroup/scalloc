// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_CORE_CACHE_H_
#define SCALLOC_CORE_CACHE_H_

#include "common.h"

#include <sched.h>

#include "random.h"
#include "spinlock-inl.h"
#include "typed_allocator.h"

namespace scalloc {

class SmallAllocator;

// Core-local policy (POLICY_CORE_LOCAL):
// We allocate all core buffers in a fixed-size (MAX_CORES) array and use
// sched_getcpu (on Linux) to assign an appropriate buffer. Note that this
// requires a concurrent implementation of the underlying allocator, since a
// thread may get descheduled while allocating into a core-local buffer.
class CoreCache {
 public:
  static const uint64_t kMaxCores = 160;

  static void Init(TypedAllocator<CoreCache>* cache_alloc);
  static CoreCache& GetCache();

  inline SmallAllocator* Allocator() { return alloc_; }

 private:
  static CoreCache* NewIfNecessary(uint64_t id);

  static uint64_t num_cores_;
  static CoreCache* caches_[kMaxCores];
  static SpinLock core_lock_;

  SmallAllocator* alloc_;
};


inline CoreCache& CoreCache::GetCache() {
  uint64_t core_id;
#ifdef __APPLE__
  core_id = hwrand() % num_cores_;
#endif
#ifdef __linux__
  core_id = static_cast<uint64_t>(sched_getcpu());
#endif
  LockScope(core_lock_);
  CoreCache* cache = caches_[core_id];
  if (LIKELY(cache != NULL)) {
    return *cache;
  }
  cache = NewIfNecessary(core_id);
  ScallocAssert(cache != NULL);
  return *cache;
}

}  // namespace scalloc

#endif  // SCALLOC_CORE_CACHE_H_

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "core_cache.h"

#include "allocators/small_allocator.h"
#include "spinlock-inl.h"
#include "utils.h"

namespace {

cache_aligned SpinLock core_cache_lock(LINKER_INITIALIZED);
cache_aligned uint64_t core_id;
scalloc::TypedAllocator<scalloc::CoreCache>* cache_allocator;

}  // namespace

namespace scalloc {

uint64_t CoreCache::num_cores_;
CoreCache* CoreCache::caches_[kMaxCores];
  
void CoreCache::Init(TypedAllocator<CoreCache>* cache_alloc) {
  num_cores_ = utils::Cpus();
  cache_allocator = cache_alloc;
  for(int i = 0 ; i < kMaxCores; i++) {
    caches_[i] = NULL;
  }
  core_id = 0;
}
  
  
CoreCache* CoreCache::NewIfNecessary(uint64_t id) {
  LockScope(core_cache_lock);

  if (caches_[id] == NULL) {
    CoreCache* cc = cache_allocator->New();
    cc->alloc_ = SmallAllocator::New(core_id++);
    caches_[id] = cc;
  }
  return caches_[id];
}

}  // namespace scalloc

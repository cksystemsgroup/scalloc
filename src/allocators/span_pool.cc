// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/span_pool.h"

#include "scalloc_arenas.h"
#include "common.h"
#include "log.h"
#include "runtime_vars.h"
#include "system-alloc.h"

namespace scalloc {

SpanPool SpanPool::page_heap_ cache_aligned;
__thread TLS_MODE size_t SpanPool::refill_ cache_aligned;

cache_aligned size_t global_refill;
cache_aligned SpinLock refill_lock_(LINKER_INITIALIZED);

void SpanPool::InitModule() {
  //page_heap_.page_pool_.Init(kSpanPoolBackends);
  unsigned num_cores = RuntimeVars::Cpus();
  for (unsigned i = 0; i < kNumClasses; ++i) {
    page_heap_.size_class_pool_[i].Init(num_cores);
  }
}

void* SpanPool::RefillOne() {
  //const size_t block_size = RuntimeVars::SystemPageSize() * kPageMultiple;
  const size_t block_size = kVirtualSpanSize;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(SmallArena.Allocate(block_size));
  void* result = reinterpret_cast<void*>(ptr);
  return result;
}

void SpanPool::Refill(const size_t refill) {
  return;
  /*
  const size_t block_size = RuntimeVars::SystemPageSize() * kPageMultiple;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(
      SmallArena.Allocate(refill * block_size));
  for (size_t i = 0; i < refill; i++) {
    Put(reinterpret_cast<void*>(ptr));
    ptr += block_size;
  }
  */
}

}  // namespace scalloc

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/span_pool.h"

#include "scalloc_arenas.h"
#include "common.h"
#include "log.h"
#include "system-alloc.h"
#include "utils.h"

namespace scalloc {

SpanPool SpanPool::page_heap_ cache_aligned;
cache_aligned size_t global_refill;
cache_aligned SpinLock refill_lock_(LINKER_INITIALIZED);

void SpanPool::InitModule() {
  unsigned num_cores = utils::Cpus();
  for (unsigned i = 0; i < kNumClasses; ++i) {
    page_heap_.size_class_pool_[i].Init(num_cores);
  }
}

void* SpanPool::RefillOne() {
  const size_t block_size = kVirtualSpanSize;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(SmallArena.Allocate(block_size));
  void* result = reinterpret_cast<void*>(ptr);
  return result;
}

}  // namespace scalloc

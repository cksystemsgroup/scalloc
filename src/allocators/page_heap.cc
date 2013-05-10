// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/page_heap.h"

#include "common.h"
#include "log.h"
#include "runtime_vars.h"
#include "system-alloc.h"

namespace scalloc {

PageHeap PageHeap::page_heap_ cache_aligned;

void PageHeap::InitModule() {
  DistributedQueue::InitModule();

  page_heap_.page_pool_.Init(kPageHeapBackends);
}


void* PageHeap::AsyncRefill(const size_t refill) {
  const size_t block_size = RuntimeVars::SystemPageSize() * kPageMultiple;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(SystemAlloc_Mmap(
      block_size * refill, NULL));
  if (UNLIKELY(ptr == 0)) {
    ErrorOut("SystemAlloc failed");
  }
  void* result = reinterpret_cast<void*>(ptr);
  ptr += block_size;
  for (size_t i = 1; i < refill; i++) {
    Put(reinterpret_cast<void*>(ptr));
    ptr += block_size;
  }
  return result;
}

}  // namespace scalloc

// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/span_pool.h"

#include "buffer/lab.h"
#include "common.h"
#include "log.h"
#include "scalloc_arenas.h"
#include "utils.h"

namespace scalloc {

extern cache_aligned RRAllocationBuffer ab;
cache_aligned SpanPool SpanPool::span_pool_;


void SpanPool::Init() {
  for (unsigned i = 0; i < kNumClasses; ++i) {
    span_pool_.size_class_pool_[i].Init(utils::Parallelism());
  }
}


void* SpanPool::RefillOne() {
  return SmallArena.Allocate(kVirtualSpanSize);
}


void* SpanPool::Refill(uint64_t sc, uint32_t tid) {
  uint64_t num = ab.GetAllocationBuffer(NULL).NextSpanRefill(sc);
  uint64_t sz  = num * kVirtualSpanSize;
  void* p = SmallArena.Allocate(sz);

  //LOG(kWarning, "tid: %u sc: %lu num %lu", tid, sc, num);

  uintptr_t start = reinterpret_cast<uintptr_t>(p) + kVirtualSpanSize;
  void* start_saved = reinterpret_cast<void*>(start);
  for (uint64_t i = 2; i < num; i++) {
    *(reinterpret_cast<void**>(start)) = reinterpret_cast<void*>(start + kVirtualSpanSize);
    start += kVirtualSpanSize;
  }
  if (num > 1) {
    size_class_pool_[sc].EnqueueBufferAt(start_saved, reinterpret_cast<void*>(start), tid % utils::Parallelism());
  }
  return p;
}

}  // namespace scalloc

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SPAN_POOL_H_
#define SCALLOC_ALLOCATORS_SPAN_POOL_H_

#include <sys/mman.h>  // madvise

#include "common.h"
#include "distributed_queue.h"
#include "size_map.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

class SpanPool {
 public:
  static void InitModule();
  static SpanPool& Instance();

  void Refill(const size_t refill);
  void* Get(size_t sc);
  void Put(void* p, size_t sc);

 private:
  static SpanPool page_heap_ cache_aligned;
  static const size_t kSpanPoolBackends = kNumClasses;
  static size_t __thread refill_ cache_aligned;

  DistributedQueue page_pool_ cache_aligned;

  void* RefillOne();
};

always_inline SpanPool& SpanPool::Instance() {
  return page_heap_;
}

always_inline void SpanPool::Put(void* p, size_t sc) {
  LOG(kTrace, "[SpanPool]: put: %p", p);
  /*
  if (sc > kMaxSmallShift) {
    madvise(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + 4096), (1UL << 28) - 4096, MADV_DONTNEED);
  }
  */
  page_pool_.EnqueueAt(p, sc);
#ifdef PROFILER_ON
  Profiler::GetProfiler().DecreaseRealSpanFragmentation(sc, SizeMap::Instance().MaxObjectsPerClass(sc) * SizeMap::SizeToBlockSize(sc));
#endif  // PROFILER_ON
}

always_inline void* SpanPool::Get(size_t sc) {
  LOG(kTrace, "[SpanPool]: get request");
#ifdef PROFILER_ON
  Profiler::GetProfiler().IncreaseRealSpanFragmentation(sc, SizeMap::Instance().MaxObjectsPerClass(sc) * SizeMap::SizeToBlockSize(sc));
#endif  // PROFILER_ON
  int index;
  size_t i;
  void* result;
  for (i = 0; i < kNumClasses; i++) {
    index = sc - i;
    if (index < 0) {
      index += kNumClasses;
    }
    result = page_pool_.DequeueOnlyAt(index);
    if (result != NULL) {
      break;
    }
  }
  if (result != NULL && (i > sc)) {
    madvise(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(result) + SizeMap::Instance().ClassToSpanSize(sc)),
                                    (1UL << 28) - SizeMap::Instance().ClassToSpanSize(sc), 
                                    MADV_DONTNEED);
  }
  if (UNLIKELY(result == NULL)) {
    return RefillOne();
  }

  /*
  void* result = page_pool_.Dequeue();
  if (UNLIKELY(result == NULL)) {
    return RefillOne();
  }
  LOG(kTrace, "[SpanPool]: get: %p", result);
  */
  return result;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SPAN_POOL_H_

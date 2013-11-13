// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SPAN_POOL_H_
#define SCALLOC_ALLOCATORS_SPAN_POOL_H_

#include <sys/mman.h>  // madvise

#include "common.h"
#include "distributed_queue.h"
#include "headers.h"
#include "size_classes.h"
#include "utils.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

class SpanPool {
 public:
  static void Init();
  static inline SpanPool& Instance() { return span_pool_; }

  SpanHeader* Get(size_t sc, uint32_t tid, bool* reusable);
  void Put(SpanHeader* p, size_t sc, uint32_t tid);

 private:
  void* RefillOne();

  static SpanPool span_pool_;

  DistributedQueue size_class_pool_[kNumClasses] cache_aligned;
};


inline void SpanPool::Put(SpanHeader* p, size_t sc, uint32_t tid) {
  LOG(kTrace, "[SpanPool] put: %p", p);

#ifdef EAGER_MADVISE
  if (ClassToSpanSize[sc] > kEagerMadviseThreshold) {
    LOG(kTrace, "[SpanPool] madvise (eager): %p, class :%lu, spansize: %lu",
        p, sc, ClassToSpanSize[sc]);
    madvise(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + kPageSize),
            kVirtualSpanSize - kPageSize,
            MADV_DONTNEED);
  }
#endif  // EAGER_MADVISE

  size_class_pool_[sc].EnqueueAt(p, tid % utils::Cpus());

#ifdef PROFILER_ON
  Profiler::GetProfiler().DecreaseRealSpanFragmentation(
      sc, SizeMap::Instance().ClassToSpanSize(sc));
  GlobalProfiler::Instance().LogSpanPoolPut(sc);
#endif  // PROFILER_ON
}


inline SpanHeader* SpanPool::Get(size_t sc, uint32_t tid, bool *reusable) {
  LOG(kTrace, "[SpanPool] get request");

#ifdef PROFILER_ON
  Profiler::GetProfiler().IncreaseRealSpanFragmentation(
      sc, SizeMap::Instance().ClassToSpanSize(sc));
  GlobalProfiler::Instance().LogSpanPoolGet(sc);
#endif  // PROFILER_ON

  int index;
  size_t i;
  void* result;
  *reusable = false;
  const int qindex = tid % utils::Cpus();

  for (i = 0; i < kNumClasses; i++) {
    index = sc - i;
    if (index < 0) {
      index += kNumClasses;
    }
    result = size_class_pool_[index].DequeueAt(qindex);
    if (result != NULL) {
      break;
    }
  }

  if (UNLIKELY(result == NULL)) {
    return reinterpret_cast<SpanHeader*>(RefillOne());
  }

  i = static_cast<size_t>(index);

#ifdef EAGER_MADVISE
  if ((i > sc) &&
      (ClassToSpanSize[i] != ClassToSpanSize[sc]) &&
      (ClassToSpanSize[sc] <= kEagerMadviseThreshold)) {
    LOG(kTrace, "[SpanPool] madvise (eager): %p, class :%lu, spansize: %lu",
        reinterpret_cast<void*>(result), sc, ClassToSpanSize[sc]);
#else
  if ((i > sc) &&
      (ClassToSpanSize[i] != ClassToSpanSize[sc])) {
    LOG(kTrace, "[SpanPool] madvise: %p, class :%lu, spansize: %lu",
        reinterpret_cast<void*>(result), sc, ClassToSpanSize[sc]);
#endif  // EAGER_MADVISE
    madvise(reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(result) + ClassToSpanSize[sc]),
            kVirtualSpanSize - ClassToSpanSize[sc],
            MADV_DONTNEED);

#ifdef PROFILER_ON
    GlobalProfiler::Instance().LogSpanShrink(sc);
#endif  // PROFILER_ON
  }

#ifdef EAGER_MADVISE
  if ((i == sc) &&
      (ClassToSpanSize[sc] <= kEagerMadviseThreshold)) {
#else
  if (i == sc) {
#endif  // EAGER_MADVISE
    *reusable = true;
  }

  LOG(kTrace, "[SpanPool] get: %p", result);
  return reinterpret_cast<SpanHeader*>(result);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SPAN_POOL_H_

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SPAN_POOL_H_
#define SCALLOC_ALLOCATORS_SPAN_POOL_H_

#include <sys/mman.h>  // madvise

#include "common.h"
#include "distributed_queue.h"
#include "size_map.h"
#include "utils.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

class SpanPool {
 public:
  static void Init();
  static SpanPool& Instance();

  void Refill(const size_t refill);
  void* Get(size_t sc, uint32_t tid, bool* reusable);
  void Put(void* p, size_t sc, uint32_t tid);

 private:
  static SpanPool span_pool_ cache_aligned;
  static const size_t kSpanPoolBackends = kNumClasses;

  DistributedQueue size_class_pool_[kNumClasses] cache_aligned;

  void* RefillOne();
};

inline SpanPool& SpanPool::Instance() {
  return span_pool_;
}

always_inline void SpanPool::Put(void* p, size_t sc, uint32_t tid) {
  LOG(kTrace, "[SpanPool]: put: %p", p);

#ifdef EAGER_MADVISE_ON
  if (sc > kFineClasses + 3) {
    madvise(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) +
                                    kPageSize),
                                    kVirtualSpanSize -
                                    kPageSize,
                                    MADV_DONTNEED);
  }
#endif  // EAGER_MADVISE_ON

  size_class_pool_[sc-1].EnqueueAt(p, tid % utils::Cpus());
#ifdef PROFILER_ON
  Profiler::GetProfiler().DecreaseRealSpanFragmentation(
      sc, SizeMap::Instance().ClassToSpanSize(sc));
  GlobalProfiler::Instance().LogSpanPoolPut(sc);
#endif  // PROFILER_ON
}

always_inline void* SpanPool::Get(size_t sc, uint32_t tid, bool *reusable) {
  LOG(kTrace, "[SpanPool]: get request");
#ifdef PROFILER_ON
  Profiler::GetProfiler().IncreaseRealSpanFragmentation(
      sc, SizeMap::Instance().ClassToSpanSize(sc));
  GlobalProfiler::Instance().LogSpanPoolGet(sc);
#endif  // PROFILER_ON

  sc--;  // real size classe start at 1. DQ index starts at 0

  int index;
  size_t i;
  void* result;
  *reusable = false;
  int qindex = tid % utils::Cpus();

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
    return RefillOne();
  }

#ifdef EAGER_MADVISE_ON
  if ((i > sc) && (i <= kFineClasses + 3)) {
#else
  if (i > sc) {
#endif  // EAGER_MADVISE_ON

#ifdef PROFILER_ON
    GlobalProfiler::Instance().LogSpanShrink(sc);
#endif  // PROFILER_ON

    madvise(reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(result) +
            SizeMap::Instance().ClassToSpanSize(sc)),
        kVirtualSpanSize - SizeMap::Instance().ClassToSpanSize(sc),
        MADV_DONTNEED);
  }

#ifdef EAGER_MADVISE_ON
  if ((i == sc) && (i <= kFineClasses + 3)) {
#else
  if (i == sc) {
#endif  // EAGER_MADVISE_ON
    *reusable = true;
  }


  LOG(kTrace, "[SpanPool]: get: %p", result);
  return result;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SPAN_POOL_H_

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SPAN_POOL_H_
#define SCALLOC_ALLOCATORS_SPAN_POOL_H_

#include <sys/mman.h>  // madvise

#include "common.h"
#include "distributed_queue.h"
#include "headers.h"
#include "scalloc_assert.h"
#include "size_classes.h"
#include "utils.h"

namespace scalloc {

class SpanPool {
 public:
  static void Init();
  static inline SpanPool& Instance() { return span_pool_; }

  inline SpanPool() {}
  SpanHeader* Get(size_t sc, uint32_t tid);
  void Put(SpanHeader* p, size_t sc, uint32_t tid);

 private:
  static SpanPool span_pool_;

  void* RefillOne();

  DistributedQueue size_class_pool_[kNumClasses];

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(SpanPool);
};


inline void SpanPool::Put(SpanHeader* p, size_t sc, uint32_t tid) {
  LOG_CAT("span-pool", kTrace, "put: %p", p);

#ifdef EAGER_MADVISE
  if (ClassToSpanSize[sc] > kEagerMadviseThreshold) {
    LOG_CAT("span-pool", kTrace,
            "madvise (eager): %p, class :%lu, spansize: %lu",
            p, sc, ClassToSpanSize[sc]);
    madvise(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + kPageSize),
            kVirtualSpanSize - kPageSize,
            MADV_DONTNEED);
  }
#endif  // EAGER_MADVISE

  size_class_pool_[sc].EnqueueAt(p, tid % utils::Parallelism());
}


inline SpanHeader* SpanPool::Get(size_t sc, uint32_t tid) {
  LOG_CAT("span-pool", kTrace, "get request");
  void* result;
  int index;
  size_t i;
  bool reusable = false;
  const int qindex = tid % utils::Parallelism();

  for (i = 0; i < kNumClasses; i++) {
    index = sc - i;
    if (index < 0) {
      index += kNumClasses;
    }
    result = size_class_pool_[index].DequeueStartAt(qindex);
    if (result != NULL) {
      break;
    }
  }

  if (UNLIKELY(result == NULL)) {
    SpanHeader* hdr = reinterpret_cast<SpanHeader*>(RefillOne());
    hdr->Init(sc, tid, false);
    return hdr;
  }

  i = static_cast<size_t>(index);

#ifdef EAGER_MADVISE
  if ((i > sc) &&
      (ClassToSpanSize[i] != ClassToSpanSize[sc]) &&
      (ClassToSpanSize[sc] <= kEagerMadviseThreshold)) {
    LOG_CAT("span-pool", kTrace,
            "madvise (eager): %p, class :%lu, spansize: %lu",
            reinterpret_cast<void*>(result), sc, ClassToSpanSize[sc]);
#else
  if ((i > sc) &&
      (ClassToSpanSize[i] != ClassToSpanSize[sc])) {
    LOG_CAT("span-pool", kTrace,
            "madvise: %p, class :%lu, spansize: %lu",
            reinterpret_cast<void*>(result), sc, ClassToSpanSize[sc]);
#endif  // EAGER_MADVISE
    madvise(reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(result) + ClassToSpanSize[sc]),
            kVirtualSpanSize - ClassToSpanSize[sc],
            MADV_DONTNEED);
  }

#ifdef EAGER_MADVISE
  if ((i == sc) &&
      (ClassToSpanSize[sc] <= kEagerMadviseThreshold)) {
#else
  if (i == sc) {
#endif  // EAGER_MADVISE
#ifdef REUSE_FREE_LIST
    reusable = true;
#endif  // REUSE_FREE_LIST
  }

  LOG_CAT("span-pool", kTrace, "get: %p", result);
  SpanHeader* hdr = reinterpret_cast<SpanHeader*>(result);
  hdr->Init(sc, tid, reusable);
  return hdr;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SPAN_POOL_H_

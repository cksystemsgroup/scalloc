// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SPAN_POOL_H_
#define SCALLOC_ALLOCATORS_SPAN_POOL_H_

#include <sys/mman.h>  // madvise

#include "common.h"
#include "distributed_queue.h"
#include "headers.h"
#include "platform/assert.h"
#include "size_classes.h"
#include "utils.h"

namespace scalloc {

class SpanPool {
 public:
  static void Init();
  static inline SpanPool& Instance() {
    return span_pool_;
  }

  inline SpanPool() {}
  SpanHeader* Get(size_t sc, uint32_t tid);
  void Put(SpanHeader* p, size_t sc, uint32_t tid);

 private:
  static SpanPool span_pool_;

  void* RefillOne();
  void* Refill(uint64_t sc, uint32_t tid);

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
    p->madvised = 1;
    madvise(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + kPageSize),
            kVirtualSpanSize - kPageSize,
            MADV_DONTNEED);
  }
#endif  // EAGER_MADVISE

  size_class_pool_[sc].EnqueueAt(p, tid % utils::Parallelism());
}


inline SpanHeader* SpanPool::Get(size_t sc, uint32_t tid) {
  LOG_CAT("span-pool", kTrace, "get request");
  void* result = NULL;
  bool reusable = false;
  const int qindex = tid % utils::Parallelism();
  SpanHeader * hdr;

  int index;
  for (size_t _i = 0; (_i < kNumClasses) && (result == NULL); _i++) {
    index = sc - _i;
    if (index < 0) {
      index += kNumClasses;
    }
    result = size_class_pool_[index].DequeueStartAt(qindex);
  }

  if (UNLIKELY(result == NULL)) {
    hdr = reinterpret_cast<SpanHeader*>(Refill(sc, tid));
    hdr->Init(sc, tid, false);
    return hdr;
  }
  hdr = reinterpret_cast<SpanHeader*>(result);

#if !defined(HUGE_PAGE)
  // Let's do lazy madvising.
  if ((static_cast<size_t>(index) > sc) &&
      (ClassToSpanSize[index] != ClassToSpanSize[sc]) &&
      (hdr->madvised == 0)) {
    madvise(reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(result) + ClassToSpanSize[sc]),
            kVirtualSpanSize - ClassToSpanSize[sc],
            MADV_DONTNEED);
  }
#endif  // !HUGE_PAGE

#if !defined(EAGER_MADVISE) && defined(REUSE_FREE_LIST)
  // With lazy madvising we can skip freelist initialization at the expense of
  // sequential locality.
  if (static_cast<size_t>(index) == sc) {
    reusable = true;
  }
#endif  // !EAGER_MADVISE && REUSE_FREE_LIST

  LOG_CAT("span-pool", kTrace, "get: %p", result);
  hdr->Init(sc, tid, reusable);
  return hdr;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SPAN_POOL_H_

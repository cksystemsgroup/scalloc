// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SPAN_POOL_H_
#define SCALLOC_SPAN_POOL_H_

#include <sys/mman.h>

#include <atomic>

#include "arena.h"
#include "globals.h"
#include "lock.h"
#include "size_classes.h"
#include "stack.h"

#define EAGER_MADVISE 1

namespace scalloc {

class SpanPool {
 public:
  always_inline SpanPool();
  always_inline void* Allocate(size_t size_class, int32_t id);
  always_inline void Free(size_t size_class, void* p, int32_t id);

  always_inline void AnnounceNewThread();
  always_inline void AnnounceLeavingThread();

#ifdef PROFILE
  inline void PrintProfileSummary() {
    LOG(kWarning,
        "span pool: allocations: %d,  deallocations: %d",
        nr_allocate_.load(), nr_free_.load());
  }
#endif  // PROFILE

 private:
  static const int32_t kSizeClassSlots = kCoarseClasses + 1;

  typedef Stack<64> Backend;

  always_inline int32_t limit() { return limit_.load(); }

  std::atomic<int32_t> limit_;
  std::atomic<int32_t> current_;
  uint8_t pad_[64 - ((sizeof(limit_) + sizeof(current_))  % 64)];  // NOLINT

  Backend* spans_[kSizeClassSlots];

#ifdef PROFILE
  std::atomic<int32_t> nr_allocate_;
  std::atomic<int32_t> nr_free_;
  std::atomic<int32_t> nr_madvise_;
#endif  // PROFILE
};


SpanPool::SpanPool() {
  limit_.store(1);
  for (size_t i = 0; i < kSizeClassSlots; i++) {
    spans_[i] = reinterpret_cast<Backend*>(
        SystemMmapFail(sizeof(Backend) * CpusOnline()));
  }
}


void SpanPool::AnnounceNewThread() {
  const int_fast32_t cpus = CpusOnline();
  int32_t old_limit;
  int32_t new_limit = current_.fetch_add(1);
  do {
    old_limit = limit();
    if ((new_limit >= cpus) || (new_limit <= old_limit))  {
      return;
    }
  } while (!limit_.compare_exchange_weak(old_limit, old_limit + 1));
}


void SpanPool::AnnounceLeavingThread() {
  current_.fetch_sub(1);
}


void*  SpanPool::Allocate(size_t size_class, int32_t id) {
#ifdef PROFILE
  nr_allocate_.fetch_add(1);
#endif  // PROFILE
  LOG(kTrace, "allocate size class: %lu, limit: %d, id: %d",
      size_class, limit(), id);
  if (size_class  <= kFineClasses) {
    size_class = 0;
  } else {
    size_class -= kFineClasses;
  }
  int32_t i = size_class;
  void* s = spans_[size_class][id % limit()].Pop();
  for (size_t _i = 0; (s == NULL) && (_i < kSizeClassSlots); _i++) {
    i = size_class - _i;
    if (i < 0) { i += kSizeClassSlots; }

    const uint64_t start = hwrand() % limit();
    LOG(kTrace, "start: %lu", start);
    for (int_fast32_t _j = 0; (_j < limit()) && (s == NULL); _j++) {
      s = spans_[i][(start + _j) % limit()].Pop();
    }
  }

  if (s == NULL) {
    return object_space.AllocateVirtualSpan();
  }

#ifndef EAGER_MADVISE
  // Madvising
  if ((static_cast<size_t>(i) > size_class) &&
      (ClassToSpanSize[i] < ClassToSpanSize[size_class])) {
    LOG(kWarning, "MADVISING sc old: %lu, new: %lu", size_class, i);
    madvise(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(s) + ClassToSpanSize[size_class]),
        kVirtualSpanSize - ClassToSpanSize[size_class],
        MADV_DONTNEED);
#ifdef PROFILE
    nr_madvise_.fetch_add(1);
#endif  // PROFILE
  }
#endif  // EAGER_MADVISE
  return s;
}


void SpanPool::Free(size_t size_class, void* p, int32_t id) {
  LOG(kTrace, "span pool put %p, size class: %lu", p, size_class);
#ifdef EAGER_MADVISE
  if (size_class >= 17) {
    madvise(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(p) + kPageSize),
        kVirtualSpanSize - kPageSize,
        MADV_DONTNEED);
  }
#endif  // EAGER_MADVISE
#ifdef PROFILE
  nr_free_.fetch_add(1);
#endif  // PROFILE
  if (size_class  <= kFineClasses) {
    size_class = 0;
  } else {
    size_class -= kFineClasses;
  }
  spans_[size_class][id % limit()].Push(p);
}

}  // namespace scalloc

#endif  // SCALLOC_SPAN_POOL_H_


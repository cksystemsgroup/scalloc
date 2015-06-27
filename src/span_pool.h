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

namespace scalloc {

class SpanPool {
 public:
  // Globally constructed, hence we use staged construction.
  always_inline SpanPool() {}
  always_inline ~SpanPool() {}

  always_inline void Init();
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
#if defined(SCALLOC_SPAN_POOL_BACKEND_LIMIT)
  static const int32_t kHardLimit = SCALLOC_SPAN_POOL_BACKEND_LIMIT;
#else
  static const int32_t kHardLimit = 16384;
#endif  // SCALLOC_SPAN_POOL_BACKEND_LIMIT

  typedef Stack<64> Backend;

  always_inline int32_t limit() { return limit_.load(); }

  // The currently announced number of threads.
  std::atomic<int32_t> current_threads_;

  // A monotonically increasing limit of backends that depends on the number of
  // threads.
  std::atomic<int32_t> limit_;

  UNUSED uint8_t pad_[64 - ((sizeof(limit_) + sizeof(current_threads_))  % 64)];  // NOLINT

  Backend* spans_[kSizeClassSlots];

#ifdef PROFILE
  std::atomic<int32_t> nr_allocate_;
  std::atomic<int32_t> nr_free_;
  std::atomic<int32_t> nr_madvise_;
#endif  // PROFILE
};


void SpanPool::Init() {
  current_threads_ = 0;
  limit_ = 0;
#ifdef PROFILE
  nr_allocate_ = 0;
  nr_free_ = 0;
  nr_madvise_ = 0;
#endif  // PROFILE
  for (size_t i = 0; i < kSizeClassSlots; i++) {
    spans_[i] = reinterpret_cast<Backend*>(
        SystemMmapFail(sizeof(Backend) * CpusOnline()));
  }
}


void SpanPool::AnnounceNewThread() {
  //LOG(kWarning, "announce thread");
  const int_fast32_t cpus = CpusOnline();
  int32_t old_limit;
  int32_t new_limit = current_threads_.fetch_add(1) + 1;
  if ((new_limit > cpus) || (new_limit > kHardLimit)) {
    return;
  }
  do {
    old_limit = limit();
    if (new_limit <= old_limit)  {
      return;
    }
  } while (!limit_.compare_exchange_weak(old_limit, old_limit + 1));
}


void SpanPool::AnnounceLeavingThread() {
  current_threads_.fetch_sub(1);
}


void*  SpanPool::Allocate(size_t size_class, int32_t id) {
#ifdef PROFILE
  nr_allocate_.fetch_add(1);
#endif  // PROFILE
  LOG(kTrace, "allocate size class: %lu, limit: %d, id: %d",
      size_class, limit(), id);
  ScallocAssert(limit() != 0);
  size_t size_class_slot;
  if (size_class  <= kFineClasses) {
    size_class_slot = 0;
  } else {
    size_class_slot = size_class - kFineClasses;
  }
  int32_t i = size_class_slot;
  void* s = spans_[size_class_slot][id % limit()].Pop();
  for (size_t _i = 0; (s == nullptr) && (_i < kSizeClassSlots); _i++) {
    i  = size_class_slot - _i;
    if (i < 0) { i += kSizeClassSlots; }

    const uint64_t start = hwrand() % limit();
    LOG(kTrace, "start: %lu", start);
    for (int_fast32_t _j = 0; (_j < limit()) && (s == nullptr); _j++) {
      s = spans_[i][(start + _j) % limit()].Pop();
    }
  }

  if (s == NULL) {
    s =  object_space.AllocateVirtualSpan();
  } else {
#if defined(SCALLOC_MADVISE) && !defined(SCALLOC_MADVISE_EAGER)
    // madvise for any of the non-fine size classes
    if ((i > 0) && (ClassToSpanSize[i + kFineClasses] > ClassToSpanSize[size_class])) {
      madvise(
          reinterpret_cast<void*>(
              reinterpret_cast<uintptr_t>(s) + ClassToSpanSize[size_class]),
          kVirtualSpanSize - ClassToSpanSize[size_class],
          MADV_DONTNEED);
#ifdef PROFILE
      nr_madvise_.fetch_add(1);
#endif  // PROFILE
    }
#endif  // MADVISE && !MADVISE_EAGER
  }
#if defined(SCALLOC_STRICT_PROTECT)
  if (mprotect(
          s,
          ClassToSpanSize[size_class],
          PROT_READ | PROT_WRITE) != 0) {
    Fatal("mprotect failed");
  }
#endif  // SCALLOC_STRICT_PROTECT
  return s;
}


void SpanPool::Free(size_t size_class, void* p, int32_t id) {
#ifdef PROFILE
  nr_free_.fetch_add(1);
#endif  // PROFILE
  LOG(kTrace, "span pool put %p, size class: %lu", p, size_class);
  ScallocAssert(limit() != 0);
#if defined(SCALLOC_MADVISE) && defined(SCALLOC_MADVISE_EAGER)
  if (size_class >= 17) {
    madvise(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(p) + kPageSize),
        kVirtualSpanSize - kPageSize,
        MADV_DONTNEED);
#ifdef PROFILE
    nr_madvise_.fetch_add(1);
#endif  // PROFILE
  }
#endif  // MADVISE && MADVISE_EAGER
  if (size_class  <= kFineClasses) {
    size_class = 0;
  } else {
    size_class -= kFineClasses;
  }
#if defined(SCALLOC_STRICT_PROTECT)
  if (mprotect(
          reinterpret_cast<void*>(reinterpret_cast<intptr_t>(p) + kPageSize),
          kVirtualSpanSize - kPageSize,
          PROT_NONE) != 0) {
    Fatal("mprotect failed");
  }
#endif  // SCALLOC_STRICT_PROTECT
  spans_[size_class][id % limit()].Push(p);
}

}  // namespace scalloc

#endif  // SCALLOC_SPAN_POOL_H_


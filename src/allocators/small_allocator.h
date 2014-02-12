// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SMALL_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_SMALL_ALLOCATOR_H_

#include <stdint.h>

#include "assert.h"
#include "allocators/arena.h"
#include "allocators/block_pool.h"
#include "collector.h"
#include "common.h"
#include "headers.h"
#include "list-inl.h"
#include "span_pool.h"
#include "size_classes.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

class SmallAllocator {
 public:
  static void Init(TypedAllocator<SmallAllocator>* alloc);
  static inline bool Enabled() { return enabled_; }
  static SmallAllocator* New(const uint64_t id);
  static void Destroy(SmallAllocator* thiz);

  void* Allocate(const size_t size);
  void Free(void* p, SpanHeader* hdr);

 private:
  void AddCoolSpan(const size_t sc, SpanHeader* span);
  void RemoveCoolSpan(const size_t sc, SpanHeader* span);
  void AddSlowSpan(const size_t sc, SpanHeader* span);
  void RemoveSlowSpan(const size_t sc, SpanHeader* node);
  void* AllocateNoSlab(const size_t sc, const size_t size);
  SpanHeader* InitSlab(uintptr_t block, size_t len, const size_t sc);
  void Refill(const size_t sc);
  void SetActiveSlab(const size_t sc, const SpanHeader* hdr);

  static TypedAllocator<SmallAllocator>* allocator;
  static bool enabled_;

  uint64_t id_;
  uint64_t me_active_;
  uint64_t me_inactive_;
  SpanHeader* hot_span_[kNumClasses];
  SpanHeader* cool_spans_[kNumClasses];
  ListNode* slow_spans_[kNumClasses];

  TypedAllocator<ListNode> node_allocator;

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(SmallAllocator);
} cache_aligned;


inline void SmallAllocator::AddCoolSpan(const size_t sc,
                                        SpanHeader* span) {
  LOG(kTrace, "[SmallAllocator] adding to list of cool spans %p", span);
  span->prev = NULL;
  span->next = cool_spans_[sc];
  if (cool_spans_[sc] != NULL) {
    cool_spans_[sc]->prev = span;
  }
  cool_spans_[sc] = span;
}


inline void SmallAllocator::RemoveCoolSpan(const size_t sc,
                                           SpanHeader* span) {
  LOG(kTrace, "[SmallAllocator] removing from list of cool spans %p", span);
  if (span->prev != NULL) {
    reinterpret_cast<SpanHeader*>(span->prev)->next = span->next;
  }
  if (span->next != NULL) {
    reinterpret_cast<SpanHeader*>(span->next)->prev = span->prev;
  }
  if (cool_spans_[sc] == span) {
    cool_spans_[sc] = reinterpret_cast<SpanHeader*>(span->next);
  }
  span->prev = NULL;
  span->next = NULL;
}


inline void SmallAllocator::AddSlowSpan(const size_t sc, SpanHeader* span) {
  ListNode* n = node_allocator.New();
  n->prev = NULL;
  n->data = reinterpret_cast<void*>(span);
  n->next = slow_spans_[sc];
  if (slow_spans_[sc] != NULL) {
    slow_spans_[sc]->prev = n;
  }
  slow_spans_[sc] = n;
}


inline void SmallAllocator::RemoveSlowSpan(const size_t sc, SpanHeader* span) {
  ListNode* n = slow_spans_[sc];
  while (n != NULL) {
    if (n->data == span) {
      if (n->prev != NULL) {
        n->prev->next = n->next;
      } else {
        slow_spans_[sc] = n->next;
      }
      if (n->next != NULL) {
        n->next->prev = n->prev;
      }
      node_allocator.Delete(n);
      return;
    }
    n = n->next;
  }
  UNREACHABLE();
}


inline void SmallAllocator::SetActiveSlab(const size_t sc,
                                          const SpanHeader* hdr) {
  // Prepend current hot span to the list of cool spans.
  if (hot_span_[sc] != NULL) {
    LOG(kTrace, "{%lu} hot span -> cool span %p, utilization: %lu",
        sc, hot_span_[sc], hot_span_[sc]->Utilization());
    AddCoolSpan(sc, hot_span_[sc]);
  }

  hot_span_[sc] = const_cast<SpanHeader*>(hdr);
}


inline void* SmallAllocator::Allocate(const size_t size) {
  void* result;
  const size_t sc = SizeToClass(size);
  SpanHeader* hdr = hot_span_[sc];

  if (UNLIKELY(hdr == NULL)) {
    return AllocateNoSlab(sc, size);
  }

  if ((result = hdr->flist.Pop()) != NULL) {
    LOG(kTrace, "[SmallAllocator] returning object from active span."
                " utilization: %lu", hdr->Utilization());
#ifdef PROFILER_ON
    Profiler::GetProfiler().LogAllocation(size);
#endif  // PROFILER_ON
    return result;
  }

  return AllocateNoSlab(sc, size);
}


inline void SmallAllocator::Free(void* p, SpanHeader* hdr) {
  SpanHeader* const cur_sc_hdr = hot_span_[hdr->size_class];
  const size_t sc = hdr->size_class;

  // p may be an address returned by posix_memalign(). We need to fix this.
  // |---SpanHeader---|---block---|---block---|---...---|
  // p may be anywhere in a block (because of memalign), hence we need to map it
  // back to its block starting address.
  const uintptr_t blocksize_fixed_p =
      reinterpret_cast<uintptr_t>(p) - hdr->flist_aligned_blocksize_offset;
  p = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(p) - (blocksize_fixed_p % ClassToSize[sc]));

  if (hdr->aowner.raw == me_active_) {
      // Local free for the currently used span.
#ifdef PROFILER_ON
      Profiler::GetProfiler().LogDeallocation(sc);
#endif  // PROFILER_ON
      hdr->flist.Push(p);
      LOG(kTrace, "[SmallAllocator] free in active local block at %p, block: %p, sc: %lu, utilization: %lu",
          p, hdr, sc, hdr->Utilization());
      if (hdr != cur_sc_hdr &&
          (hdr->Utilization() < kSpanReuseThreshold)) {
        if (UNLIKELY(hdr->flist.Full())) {
          RemoveCoolSpan(sc, hdr);
#ifdef MADVISE_SAME_THREAD
          SpanPool::Instance().Put(hdr, hdr->size_class, hdr->aowner.owner);
#else
          Collector::Put(hdr);
#endif  // MADVISE_SAME_THREAD
        } else {
          RemoveCoolSpan(sc, hdr);
          AddSlowSpan(sc, hdr);
          hdr->aowner.active = false;
        }
      }
      return;
  } else if (hdr->aowner.raw == me_inactive_) {
    // Local free in already globally available span.
    if (__sync_bool_compare_and_swap(
          &hdr->aowner.raw, me_inactive_, me_active_)) {
#ifdef PROFILER_ON
      Profiler::GetProfiler().LogDeallocation(sc, false);
#endif  // PROFILER_ON
      LOG(kTrace, "[SmallAllocator] free in retired local block at %p, "
                  "sc: %lu", p, sc);
      hdr->flist.Push(p);

      if (cur_sc_hdr->Utilization() > kLocalReuseThreshold) {
        RemoveSlowSpan(sc, hdr);
        SetActiveSlab(sc, hdr);
#ifdef PROFILER_ON
        Profiler::GetProfiler().LogSpanReuse();
#endif  // PROFILER_ON
        return;
      }

      if (hdr->flist.Full()) {
        LOG(kTrace, "{%lu}  returning span: %p", sc, hdr);
        RemoveSlowSpan(sc, hdr);
#ifdef MADVISE_SAME_THREAD
        SpanPool::Instance().Put(hdr, hdr->size_class, hdr->aowner.owner);
#else
        Collector::Put(hdr);
#endif  // MADVISE_SAME_THREAD
        return;
      }

      MemoryBarrier();
      hdr->aowner.active = false;
      return;
    }
  }
#ifdef PROFILER_ON
  Profiler::GetProfiler().LogDeallocation(hdr->size_class, false, true);
#endif  // PROFILER_ON
  LOG(kTrace, "[SmallAllocator] remote free for %p, owner: %lu, me: %lu",
      p, hdr->aowner.owner, id_);
  BlockPool::Instance().Free(p, hdr->size_class, hdr->remote_flist);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SMALL_ALLOCATOR_H_

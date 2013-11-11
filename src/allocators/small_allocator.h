// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SMALL_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_SMALL_ALLOCATOR_H_

#include <stdint.h>

#include "allocators/arena.h"
#include "allocators/block_pool.h"
#include "common.h"
#include "block_header.h"
#include "heap_profiler.h"
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

  void* Allocate(const size_t size);
  void Free(void* p, SpanHeader* hdr);
  void SetActiveSlab(const size_t sc, const SpanHeader* hdr);
  void Destroy();

 private:
  void Refill(const size_t sc);
  SpanHeader* InitSlab(uintptr_t block, size_t len, const size_t sc);
  void* AllocateNoSlab(const size_t sc, const size_t size);

  static TypedAllocator<SmallAllocator>* allocator;
  static bool enabled_;

  uint64_t id_;
  SpanHeader* my_headers_[kNumClasses];
  uint64_t me_active_;
  uint64_t me_inactive_;
} cache_aligned;


inline void SmallAllocator::SetActiveSlab(const size_t sc,
                                          const SpanHeader* hdr) {
  my_headers_[sc] = const_cast<SpanHeader*>(hdr);
}


inline void* SmallAllocator::Allocate(const size_t size) {
  void* result;
  const size_t sc = SizeToClass(size);
  SpanHeader* hdr = my_headers_[sc];

  if (UNLIKELY(hdr == NULL)) {
    return AllocateNoSlab(sc, size);
  }

  if ((result = hdr->flist.Pop()) != NULL) {
#ifdef PROFILER_ON
    Profiler::GetProfiler().LogAllocation(size);
#endif  // PROFILER_ON
    return result;
  }

  return AllocateNoSlab(sc, size);
}


inline void SmallAllocator::Free(void* p, SpanHeader* hdr) {
  SpanHeader* cur_sc_hdr = my_headers_[hdr->size_class];
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
      LOG(kTrace, "[SmallAllocator] free in active local block at %p", p);
      hdr->flist.Push(p);
      if (hdr != cur_sc_hdr &&
          (hdr->Utilization() < kSpanReuseThreshold)) {
        if (UNLIKELY(hdr->flist.Full())) {
          SpanPool::Instance().Put(hdr, sc, hdr->aowner.owner);
        } else {
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
        SetActiveSlab(hdr->size_class, hdr);
#ifdef PROFILER_ON
        Profiler::GetProfiler().LogSpanReuse();
#endif  // PROFILER_ON
        return;
      }

      if (hdr->flist.Full()) {
        SpanPool::Instance().Put(hdr, hdr->size_class, hdr->aowner.owner);
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
  BlockPool::Instance().Free(p, hdr->size_class, hdr->aowner.owner);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SMALL_ALLOCATOR_H_

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_

#include <stdint.h>

#include "allocators/dq_sc_allocator.h"
#include "common.h"
#include "block_header.h"
#include "page_heap.h"
#include "size_map.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

class SlabScAllocator {
 public:
  static void InitModule();

  void Init(const uint64_t id);
  void* Allocate(const size_t size);
  void Free(void* p, SlabHeader* hdr);
  void SetActiveSlab(const size_t sc, const SlabHeader* hdr);
  void Destroy();

 private:
  uint64_t id_;
  SlabHeader* my_headers_[kNumClasses];

  void Refill(const size_t sc);
  SlabHeader* InitSlab(uintptr_t block, size_t len, const size_t sc);
  void* AllocateNoSlab(const size_t sc, const size_t size);

  uint64_t me_active_;
  uint64_t me_inactive_;
} cache_aligned;

always_inline void SlabScAllocator::SetActiveSlab(const size_t sc,
                                                  const SlabHeader* hdr) {
  my_headers_[sc] = const_cast<SlabHeader*>(hdr);
}

always_inline void* SlabScAllocator::Allocate(const size_t size) {
  const size_t sc = SizeMap::SizeToClass(size);
  SlabHeader* hdr = my_headers_[sc];
  void* result;
  if (hdr && (result = hdr->flist.Pop())) {
#ifdef PROFILER_ON
    Profiler::GetProfiler().LogAllocation(size);
#endif  // PROFILER_ON
    hdr->in_use++;
    return result;
  }
  return AllocateNoSlab(sc, size);
}

always_inline void SlabScAllocator::Free(void* p, SlabHeader* hdr) {
  if (hdr->aowner.raw == me_active_) {
      // Local free for the currently used slab block.
#ifdef PROFILER_ON
      Profiler::GetProfiler().LogDeallocation(hdr->size_class);
#endif  // PROFILER_ON
      LOG(kTrace, "[SlabAllcoator]: free in active local block at %p", p);
      hdr->in_use--;
      hdr->flist.Push(p);
      if (hdr != my_headers_[hdr->size_class] &&
          (hdr->Utilization() < kSpanReuseThreshold)) {
        hdr->aowner.active = false;
      }
      return;
  } else if (hdr->aowner.raw == me_inactive_) {
    if (__sync_bool_compare_and_swap(
          &hdr->aowner.raw, me_inactive_, me_active_)) {
#ifdef PROFILER_ON
      Profiler::GetProfiler().LogDeallocation(hdr->size_class, false);
#endif  // PROFILER_ON
      LOG(kTrace, "[SlabAllcoator]: free in retired local block at %p, "
                  "sc: %lu, in_use: %lu", p, hdr->size_class, hdr->in_use);
      hdr->in_use--;
      hdr->flist.Push(p);

      SlabHeader* cur_sc_hdr = my_headers_[hdr->size_class];
      if (cur_sc_hdr->Utilization() > kLocalReuseThreshold) {
        SetActiveSlab(hdr->size_class, hdr);
#ifdef PROFILER_ON 
        Profiler::GetProfiler().LogSpanReuse();
#endif  // PROFILER_ON
        return;
      }

      if (hdr->in_use == 0) {
        PageHeap::GetHeap()->Put(hdr);
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
  LOG(kTrace, "[SlabAllocator]: remote free for %p, owner: %lu, me: %lu",
      p, hdr->aowner.owner, id_);
  DQScAllocator::Instance().Free(p, hdr->size_class, hdr->remote_flist);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_

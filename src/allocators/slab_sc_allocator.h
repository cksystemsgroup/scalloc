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

namespace scalloc {

class SlabScAllocator {
 public:
  static void InitModule();

  void Init(const uint64_t id);
  void* Allocate(const size_t size);
  void Free(void* p, SlabHeader* hdr);
  void SetActiveSlab(const size_t sc, const SlabHeader* hdr);

 private:
  uint64_t id_;
  SlabHeader* my_headers_[kNumClasses];

  void Refill(const size_t sc);
  SlabHeader* InitSlab(uintptr_t block, size_t len, const size_t sc);
  void* AllocateNoSlab(const size_t sc, const size_t size);
} cache_aligned;

always_inline void SlabScAllocator::SetActiveSlab(const size_t sc,
                                                  const SlabHeader* hdr) {
  if (my_headers_[sc] != NULL) {
    my_headers_[sc]->active = false;
  }
  my_headers_[sc] = const_cast<SlabHeader*>(hdr);
}

always_inline void* SlabScAllocator::Allocate(const size_t size) {
  const size_t sc = SizeMap::SizeToClass(size);
  SlabHeader* hdr = my_headers_[sc];
  void* result;
  if (hdr && (result = hdr->flist.Pop())) {
    hdr->in_use++;
    return result;
  }
  return AllocateNoSlab(sc, size);
}

always_inline void SlabScAllocator::Free(void* p, SlabHeader* hdr) {
  if (hdr->owner == id_) {
    if (hdr->active) {
      // Local free for the currently used slab block.
      LOG(kTrace, "[SlabAllcoator]: free in active local block at %p", p);
      hdr->in_use--;
      hdr->flist.Push(p);
      return;
    } else if (!hdr->active &&
               (__sync_lock_test_and_set(&hdr->active, 1) == 0)) {
      LOG(kTrace, "[SlabAllcoator]: free in retired local block at %p, "
                  "sc: %lu, in_use: %lu", p, hdr->size_class, hdr->in_use);
      // lock a block (by making it active) that was previously used by the
      // current thread.
      hdr->in_use--;
      hdr->flist.Push(p);

      SlabHeader* cur_sc_hdr = my_headers_[hdr->size_class];
      if (cur_sc_hdr->Utilization() > 90 &&
          hdr->Utilization() < 90) {
        SetActiveSlab(hdr->size_class, hdr);
        return;
      }

      if (hdr->in_use == 0) {
        PageHeap::GetHeap()->Put(hdr);
        return;
      }

      __sync_lock_release(&hdr->active);
      return;
    }
  }
  DQScAllocator::Instance().Free(p, hdr->size_class, hdr->remote_flist);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_

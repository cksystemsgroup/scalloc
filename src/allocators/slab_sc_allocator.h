// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_

#include <stdint.h>

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
  void RemoteFree(void* p, SlabHeader* hdr);
};

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
  if (hdr == NULL || hdr->flist.Empty()) {
    return AllocateNoSlab(sc, size);
  }
  hdr->in_use++;
  return hdr->flist.Pop();
}

always_inline void SlabScAllocator::Free(void* p, SlabHeader* hdr) {
  if (hdr->owner == id_) {
    if (hdr->active) {
      // Local free for the currently used slab block.
      hdr->in_use--;
      hdr->flist.Push(p);
      return;
    } else if (!hdr->active &&
               __sync_bool_compare_and_swap(&hdr->active, false, true)) {
      // lock free for some slab block that was prev used by this thread
      // making the block active for the time of returning the object, so no
      // other thread can steal it.
      hdr->in_use--;
      hdr->flist.Push(p);
      if (hdr->in_use == 0) {
        PageHeap::GetHeap()->Put(hdr);
      }
      CompilerBarrier();
      hdr->active = false;
      return;
    }
  }
  RemoteFree(p, hdr);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SLAB_SC_ALLOCATOR_H_

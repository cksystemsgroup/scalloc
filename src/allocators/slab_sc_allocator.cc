// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "slab_sc_allocator.h"

#include "allocators/page_heap.h"
#include "log.h"
#include "runtime_vars.h"

namespace scalloc {

void SlabScAllocator::InitModule() {
  PageHeap::InitModule();
}

void SlabScAllocator::Init(const uint64_t id) {
  id_ = id;
  ActiveOwner dummy;
  dummy.Reset(true, id_);
  me_active_ = dummy.raw;
  dummy.Reset(false, id_);
  me_inactive_ = dummy.raw;
}

void* SlabScAllocator::AllocateNoSlab(const size_t sc, const size_t size) {
  // Size class 0 represents an object of size 0, which results in malloc()
  // returning NULL.
  if (sc == 0) {
    return NULL;
  }

  if (my_headers_[sc] != NULL) {
#ifdef PROFILER_ON
    Profiler::GetProfiler().LogAllocation(size);
#endif  // PROFILER_ON
    // Only try to steal we had a slab at least once.
    SlabHeader* hdr;
    void* p = DQScAllocator::Instance().Allocate(sc, id_, id_, &hdr);
    if (p != NULL) {
#ifdef PROFILER_ON
      Profiler::GetProfiler().LogBlockStealing();
#endif  // PROFILER_ON
      if (hdr != NULL) {
        SetActiveSlab(sc, hdr);
#ifdef PROFILER_ON
        Profiler::GetProfiler().LogSpanReuse(true);
#endif  // PROFILER_ON

      } else {
#ifdef GLOBAL_CLEANUP
        if (reinterpret_cast<SlabHeader*>(
                BlockHeader::GetFromObject(p))->aowner.owner != id_) {
          Refill(sc);
        }
#endif  // GLOBAL_CLEANUP
      }
      return p;
    }
  }

  Refill(sc);
  return Allocate(size);
}

void SlabScAllocator::Refill(const size_t sc) {
#ifdef PROFILER_ON
  Profiler::GetProfiler().LogSizeclassRefill();
#endif  // PROFILER_ON
  LOG(kTrace, "[SlabAllocator]: refilling size class: %lu", sc);
  uintptr_t block = reinterpret_cast<uintptr_t>(PageHeap::GetHeap()->Get());
  if (block == 0) {
    ErrorOut("PageHeap out of memory");
  }
  SlabHeader* hdr = InitSlab(block,
                             kPageMultiple * RuntimeVars::SystemPageSize(),
                             sc);
  SetActiveSlab(sc, hdr);
}

SlabHeader* SlabScAllocator::InitSlab(uintptr_t block,
                                      size_t len,
                                      const size_t sc) {
  const size_t obj_size = SizeMap::Instance().ClassToSize(sc);
  size_t sys_page_size = RuntimeVars::SystemPageSize();
  SlabHeader* main_hdr = reinterpret_cast<SlabHeader*>(block);
  main_hdr->Reset(sc, id_);
  main_hdr->aowner.owner = id_;
  main_hdr->aowner.active = true;

  // We need to initialize the flist and place ForwardHeaders every system page
  // size bytes, since this is where we search for headers (for small AND large
  // objects)

  size_t nr_objects;

  // First block is special, since it contains the full slab header, while
  // others only contain the forward header to the first block.
  nr_objects = (sys_page_size - sizeof(*main_hdr)) / obj_size;
  main_hdr->flist.FromBlock(reinterpret_cast<void*>(block + sizeof(*main_hdr)),
                            obj_size,
                            nr_objects);
  len -= sys_page_size;
  block += sys_page_size;

  // Fill with forward headers and flist objects.
  for (; len >= sys_page_size; len -= sys_page_size) {
    ForwardHeader* fwd_hdr = reinterpret_cast<ForwardHeader*>(block);
    fwd_hdr->Reset(main_hdr);
    nr_objects = (sys_page_size - sizeof(*fwd_hdr)) / obj_size;
    main_hdr->flist.AddRange(reinterpret_cast<void*>(block + sizeof(*fwd_hdr)),
                             obj_size,
                             nr_objects);
    block += sys_page_size;
  }

  return main_hdr;
}

void SlabScAllocator::Destroy() {
  // Destroying basically means giving up all active spans.  We can only give up
  // our current spans, since we do not have references to the others.  Assuming
  // the mutator had no memory leak (i.e. all non-shared objects have been
  // freed), a span now can stay active forever (all reusable blocks travel
  // through the remote freelists), or it is already inactive (and thus
  // available for reuse).
  for (size_t i = 0; i < kNumClasses; i++) {
    if (my_headers_[i]) {
      my_headers_[i]->aowner.active = false;
    }
  }
}

}  // namespace scalloc

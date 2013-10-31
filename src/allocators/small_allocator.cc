// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "small_allocator.h"

#include "allocators/span_pool.h"
#include "log.h"
#include "size_classes.h"

namespace scalloc {

TypedAllocator<SmallAllocator>* SmallAllocator::allocator;
bool SmallAllocator::enabled_;

void SmallAllocator::InitModule(TypedAllocator<SmallAllocator>* alloc) {
  enabled_ = true;
  allocator = alloc;
}

SmallAllocator* SmallAllocator::New() {
  return allocator->New();
}

void SmallAllocator::Init(const uint64_t id) {
  id_ = id;
  ActiveOwner dummy;
  dummy.Reset(true, id_);
  me_active_ = dummy.raw;
  dummy.Reset(false, id_);
  me_inactive_ = dummy.raw;
}

void* SmallAllocator::AllocateNoSlab(const size_t sc, const size_t size) {
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
    SpanHeader* hdr;
    void* p = BlockPool::Instance().Allocate(sc, id_, id_, &hdr);
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
        if (reinterpret_cast<SpanHeader*>(
                SpanHeader::GetFromObject(p))->aowner.owner != id_) {
          Refill(sc);
        }
      }
      return p;
    }
  }

  Refill(sc);
  return Allocate(size);
}

void SmallAllocator::Refill(const size_t sc) {
#ifdef PROFILER_ON
  Profiler::GetProfiler().LogSizeclassRefill();
#endif  // PROFILER_ON
  LOG(kTrace, "[SlabAllocator] refilling size class: %lu, object size: %lu",
      sc, ClassToSize[sc]);
  bool reusable;
  uintptr_t block = reinterpret_cast<uintptr_t>
      (SpanPool::Instance().Get(sc, id_, &reusable));
  if (UNLIKELY(block == 0)) {
    Fatal("SpanPool out of memory");
  }
  SpanHeader* hdr;
  hdr = reinterpret_cast<SpanHeader*>(block);
  hdr->Reset(sc, id_);
  hdr->aowner.owner = id_;
  hdr->aowner.active = true;
  hdr->max_num_blocks = ClassToObjects[sc];

  if (!reusable) {
    block += sizeof(SpanHeader);
    size_t block_size = ClassToSize[sc];
    hdr->flist.FromBlock(reinterpret_cast<void*>(block),
                         block_size,
                         hdr->max_num_blocks);
  }

  SetActiveSlab(sc, hdr);
}

void SmallAllocator::Destroy() {
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

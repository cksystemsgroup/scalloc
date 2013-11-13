// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "small_allocator.h"

#include "allocators/span_pool.h"
#include "log.h"
#include "size_classes.h"

namespace scalloc {

TypedAllocator<SmallAllocator>* SmallAllocator::allocator;
bool SmallAllocator::enabled_;


void SmallAllocator::Init(TypedAllocator<SmallAllocator>* alloc) {
  enabled_ = true;
  allocator = alloc;
}


SmallAllocator* SmallAllocator::New(const uint64_t id) {
  SmallAllocator* a = allocator->New();
  a->id_ = id;
  ActiveOwner dummy;
  dummy.Reset(true, id);
  a->me_active_ = dummy.raw;
  dummy.Reset(false, id);
  a->me_inactive_ = dummy.raw;
  return a;
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
    // Only try to steal we had a span at least once.
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
  LOG(kTrace, "[SmallAllocator] refilling size class: %lu, object size: %lu",
      sc, ClassToSize[sc]);
  bool reusable;
  SpanHeader* hdr = SpanPool::Instance().Get(sc, id_, &reusable);
  ScallocAssert(hdr != 0);
  hdr->Init(sc, id_, reusable);
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

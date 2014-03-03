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
  LOG(kTrace, "[SmallAllocator] New; id: %lu, p: %p", id, a);
  a->id_ = id;
  ActiveOwner dummy;
  dummy.Reset(true, id);
  a->me_active_ = dummy.raw;
  dummy.Reset(false, id);
  a->me_inactive_ = dummy.raw;
  a->node_allocator.Init(kPageSize, 64, "node_alloc");
  for (size_t i = 0; i < kNumClasses; i++) {
    a->hot_span_[i] = NULL;
    a->cool_spans_[i] = NULL;
    a->slow_spans_[i] = NULL;
  }
#ifdef POLICY_CORE_LOCAL
  a->core_lock_.Reset();
  a->locker_ = 0;
#endif  // POLICY_CORE_LOCAL
  return a;
}


void* SmallAllocator::AllocateNoSlab(const size_t sc, const size_t size) {
  // Size class 0 represents an object of size 0, which results in malloc()
  // returning NULL.
  if (sc == 0) {
    return NULL;
  }

  // if (hot_span_[sc] != NULL) {
#ifdef PROFILER_ON
    Profiler::GetProfiler().LogAllocation(size);
#endif  // PROFILER_ON
    // Only try to steal we had a span at least once.
    SpanHeader* hdr;
    void* p = BlockPool::Instance().Allocate(sc, id_, &hdr);
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
        if ((SpanHeader::GetFromObject(p)->aowner.owner % utils::Cpus()) !=
                (id_ % utils::Cpus())) {
          Refill(sc);
        }
      }
      return p;
    }
  // }

  Refill(sc);
  return Allocate(size);
}


void SmallAllocator::Refill(const size_t sc) {
#ifdef PROFILER_ON
  Profiler::GetProfiler().LogSizeclassRefill();
#endif  // PROFILER_ON
  LOG(kTrace, "[SmallAllocator] refilling size class: %lu, object size: %lu",
      sc, ClassToSize[sc]);

  // We are not checking cool spans, because their utilization is per
  // definition > kReuseThreshold, e.g. >80%.

  SpanHeader* span;

#ifdef REUSE_SLOW_SPANS
  // Try to reactivate a slow span.
  ListNode* tmp;
  ListNode* n = slow_spans_[sc];
  while (n != NULL) {
    tmp = n;
    n = n->next;
    // Remove it.
    slow_spans_[sc] = tmp->next;
    if (tmp->next != NULL) {
      tmp->next->prev = NULL;
    }
    span = reinterpret_cast<SpanHeader*>(tmp->data);
    ScallocAssert(span != NULL);
    node_allocator.Delete(tmp);
    // Try to get span.
    if (span->aowner.owner == id_ &&
        span->size_class == sc &&
        __sync_bool_compare_and_swap(&span->aowner.raw,
                                     me_inactive_,
                                     me_active_)) {
      SetActiveSlab(sc, span);
      return;
    }
  }
#endif  // REUSE_SLOW_SPANS

  // Get a span from SP.
  bool reusable = false;
  span = SpanPool::Instance().Get(sc, id_, &reusable);
  ScallocAssert(span != 0);
  span->Init(sc, id_, reusable);
  SetActiveSlab(sc, span);
  return;
}


void SmallAllocator::Destroy(SmallAllocator* thiz) {
  // Destroying basically means giving up hot and cool spans.  Remotely freed
  // blocks keep the span in the system, i.e., it is not released from the
  // allocator. This is similar to keeping a buffer of objects. Spans will
  // eventually be reused, since they are globally available, i.e., stealable.

  SpanHeader* cur;
  for (size_t i = 0; i < kNumClasses; i++) {
    // Hot spans.
    if (thiz->hot_span_[i] != NULL) {
      cur = thiz->hot_span_[i];
      if (cur->flist.Full()) {
#ifdef MADVISE_SAME_THREAD
        SpanPool::Instance().Put(cur, cur->size_class, cur->aowner.owner);
#else
        Collector::Put(cur);
#endif  // MADVISE_SAME_THREAD
      } else {
        thiz->hot_span_[i]->aowner.active = false;
      }
    }
    thiz->hot_span_[i] = NULL;

    // Cool spans.
    cur = reinterpret_cast<SpanHeader*>(thiz->cool_spans_[i]);
    SpanHeader* tmp;
    while (cur != NULL) {
      LOG(kTrace, "[SmallAllocator]: making span global %p", cur);
      tmp = cur;
      cur = reinterpret_cast<SpanHeader*>(cur->next);
      MemoryBarrier();
      tmp->next = NULL;
      tmp->prev = NULL;
      tmp->aowner.active = false;
    }
    thiz->cool_spans_[i] = NULL;

#ifdef REUSE_SLOW_SPANS
    // Slow spans.
    ListNode* tmp2;
    ListNode* n = thiz->slow_spans_[i];
    while (n != NULL) {
      tmp2 = n;
      n = n->next;
      thiz->node_allocator.Delete(tmp2);
    }
    thiz->slow_spans_[i]= NULL;
#endif  // REUSE_SLOW_SPANS
  }

  SmallAllocator::allocator->Delete(thiz);
}

}  // namespace scalloc

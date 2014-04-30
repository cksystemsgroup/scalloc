// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/block_pool.h"

#include "distributed_queue.h"
#include "headers.h"
#include "profiler.h"

#ifdef POLICY_CORE_LOCAL
#include "buffer/core.h"
#else
#include "thread_cache.h"
#endif  // POLICY_CORE_LOCAL

namespace scalloc {

cache_aligned BlockPool BlockPool::block_pool_;


void BlockPool::Init() {
  for (size_t i = 0; i < kNumClasses; i++) {
    block_pool_.dqs_[i].Init(utils::Parallelism());
  }
}


void* BlockPool::Allocate(const size_t sc,
                          const size_t tid,
                          const size_t start_at,
                          SpanHeader** block) {
  *block = NULL;
  void* p = dqs_[sc].DequeueStartAt(start_at % utils::Parallelism());
  /*
  if (p != NULL) {
    // We found an object, let's try to steal the whole block.
    SpanHeader* hdr = SpanHeader::GetFromObject(p);
    ScallocAssert(hdr != NULL);
    const ActiveOwner would_steal(false, hdr->aowner.owner);
    const ActiveOwner my(true, tid);
    if (!hdr->aowner.active &&
        __sync_bool_compare_and_swap(&hdr->aowner.raw,
                                     would_steal.raw,
                                     my.raw)) {
      // Got it!
      PROFILER_STEAL();
      *block = hdr;
      LOG(kTrace, "[BlockPool] steal successful: %p", hdr);
    }
  }
  */

  return p;
}

}  // namespace scalloc

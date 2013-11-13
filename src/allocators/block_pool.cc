// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/block_pool.h"

#include "distributed_queue.h"
#include "headers.h"

namespace scalloc {

cache_aligned BlockPool BlockPool::block_pool_;


void BlockPool::Init() {
  for (size_t i = 0; i < kNumClasses; i++) {
    block_pool_.dqs_[i].Init(utils::Cpus());
  }
}


void* BlockPool::Allocate(const size_t sc,
                              const size_t dq_id,
                              const size_t tid,
                              SpanHeader** block) {
  void* p = dqs_[sc].DequeueAt(dq_id % utils::Cpus());
  *block = NULL;

  if (p != NULL) {
    // We found an object, let's try to steal the whole block.
    SpanHeader* hdr = reinterpret_cast<SpanHeader*>(
        SpanHeader::GetFromObject(p));
    ActiveOwner would_steal(false, hdr->aowner.owner);
    ActiveOwner my(true, tid);
    if (!hdr->aowner.active &&
        __sync_bool_compare_and_swap(&hdr->aowner.raw,
                                     would_steal.raw,
                                     my.raw)) {
      // Got it!
      *block = hdr;
    }
  }

  return p;
}

}  // namespace scalloc

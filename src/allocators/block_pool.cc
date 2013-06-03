// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/block_pool.h"

#include "block_header.h"
#include "distributed_queue.h"
#include "runtime_vars.h"

namespace scalloc {

void BlockPool::InitModule() {
  Instance().Init();
}

void BlockPool::Init() {
  for (size_t i = 0; i < kNumClasses; i++) {
    dqs_[i].Init(RuntimeVars::Cpus());
  }
}

void* BlockPool::Allocate(const size_t sc,
                              const size_t dq_id,
                              const size_t tid,
                              SlabHeader** block) {
  void* p = dqs_[sc].DequeueAt(dq_id % RuntimeVars::Cpus());
  *block = NULL;

  if (p != NULL) {
    // We found an object, let's try to steal the whole block.
    SlabHeader* hdr = reinterpret_cast<SlabHeader*>(
        BlockHeader::GetFromObject(p));
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

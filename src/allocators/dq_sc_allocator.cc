// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/dq_sc_allocator.h"

#include "block_header.h"
#include "distributed_queue.h"
#include "runtime_vars.h"

namespace scalloc {

void DQScAllocator::InitModule() {
  DistributedQueue::InitModule();
  Instance().Init();
}

void DQScAllocator::Init() {
  for (size_t i = 0; i < kNumClasses; i++) {
    dqs_[i].Init(RuntimeVars::Cpus());
  }
}

void* DQScAllocator::Allocate(const size_t sc,
                              const size_t dq_id,
                              const size_t tid,
                              SlabHeader** block) {
  void* p = dqs_[sc].DequeueAt(dq_id % RuntimeVars::Cpus());
  *block = NULL;

  if (p != NULL) {
    // We found an object, let's try to steal the whole block.
    SlabHeader* hdr = reinterpret_cast<SlabHeader*>(
        BlockHeader::GetFromObject(p));
    if (hdr->active == false &&  // block is not in use
        hdr->flist.Size() > 100 &&  // enough free objects to account
                                   // for this stealing procedure
        __sync_lock_test_and_set(&hdr->active, 1) == 0) {  // try to steal it
        // Got it!
        *block = hdr;
        hdr->owner = tid;
    }
  }

  return p;
}

}  // namespace scalloc

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/dq_sc_allocator.h"

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
                              bool* steal_failed) {
  void* p = dqs_[sc].DequeueAt(dq_id % RuntimeVars::Cpus());

  // TODO(mlippautz): implement block stealing here

  return p;
}

}  // namespace scalloc

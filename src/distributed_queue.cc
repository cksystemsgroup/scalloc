// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "distributed_queue.h"

#include "common.h"
#include "runtime_vars.h"
#include "spinlock-inl.h"

scalloc::PageHeapAllocator<Stack, 64> DistributedQueue::backend_allocator_;
scalloc::PageHeapAllocator<DistributedQueue::State, 64>
    DistributedQueue::state_allocator_;
__thread DistributedQueue::State* DistributedQueue::state_;

void DistributedQueue::InitModule() {
  backend_allocator_.Init(RuntimeVars::SystemPageSize());
  state_allocator_.Init(RuntimeVars::SystemPageSize());
}

void DistributedQueue::Init(size_t p) {
  p_ = p;
  for (size_t i = 0; i < p_; i++) {
    backends_[i] = backend_allocator_.New();
    backends_[i]->Init();
  }
}

DistributedQueue::State* DistributedQueue::NewState() {
  return state_allocator_.New();
}

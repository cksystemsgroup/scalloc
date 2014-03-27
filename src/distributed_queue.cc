// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "distributed_queue.h"

namespace scalloc {

TypedAllocator<DistributedQueue::Backend>* DistributedQueue::backend_allocator_;


void DistributedQueue::Init(TypedAllocator<Backend>* backend_alloc) {
  backend_allocator_ = backend_alloc;
}


void DistributedQueue::Init(size_t p) {
  if (p > kMaxBackends) {
    p = kMaxBackends;
  }
  p_ = p;
  for (size_t i = 0; i < p_; i++) {
    backends_[i] = backend_allocator_->New();
    backends_[i]->Init();
  }
}

}  // namespace scalloc


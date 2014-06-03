// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "distributed_queue.h"

#include "utils.h"

namespace scalloc {

TypedAllocator<DistributedQueue::Backend>* DistributedQueue::backend_allocator_;


void DistributedQueue::Init(TypedAllocator<Backend>* backend_alloc) {
  backend_allocator_ = backend_alloc;
}


void DistributedQueue::Init(size_t p) {
  if (!utils::IsPowerOfTwo(p)) {
    size_t new_p = 1 << (utils::Log2(p) + 1);
    LOG_CAT("distributed-queue", kInfo,
            "#partial queues not power of 2 (wanted: %d). initializing with p=%d",
            p, new_p);
    p = new_p;
  }
  if (p > kMaxBackends) {
    LOG_CAT("distributed-queue", kInfo,
            "#partial queues to large (wanted: %d). initializing with p=%d (max)",
            p, kMaxBackends);
    p = kMaxBackends;
  }
  p_ = p;
  p_mask_ = p_ - 1;
  for (size_t i = 0; i < p_; i++) {
    backends_[i] = backend_allocator_->New();
    backends_[i]->Init();
  }
}

}  // namespace scalloc


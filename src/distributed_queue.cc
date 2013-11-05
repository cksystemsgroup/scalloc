// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "distributed_queue.h"

#include "common.h"
#include "spinlock-inl.h"

namespace scalloc {

TypedAllocator<DistributedQueue::Backend>* DistributedQueue::backend_allocator_;
TypedAllocator<DistributedQueue::State>* DistributedQueue::state_allocator_;
#ifdef HAVE_TLS
__thread DistributedQueue::State* DistributedQueue::state_;
#else
pthread_key_t DistributedQueue::state_key_;
#endif


void DistributedQueue::Init(TypedAllocator<State>* state_alloc,
                            TypedAllocator<Backend>* backend_alloc) {
  backend_allocator_ = backend_alloc;
  state_allocator_ = state_alloc;
#ifndef HAVE_TLS
  pthread_key_create(&state_key_, NULL);
#endif
}


void DistributedQueue::Init(size_t p) {
  p_ = p;
  for (size_t i = 0; i < p_; i++) {
    backends_[i] = backend_allocator_->New();
    backends_[i]->Init();
  }
}


DistributedQueue::State* DistributedQueue::NewState() {
  State* state = state_allocator_->New();
#ifdef HAVE_TLS
  state_ = state;
#else
  pthread_setspecific(state_key_, static_cast<void*>(state));
#endif
  return state;
}


DistributedQueue::State* DistributedQueue::GetState() {
#ifdef HAVE_TLS
  return state_;
#else
  return static_cast<State*>(pthread_getspecific(state_key_));
#endif
}

}  // namespace scalloc

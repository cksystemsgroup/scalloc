// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "distributed_queue.h"

#include "common.h"

namespace {
  scalloc::TypedAllocator<scalloc::DistributedQueue::State>* state_allocator;
}  // namespace

namespace scalloc {

#ifdef HAVE_TLS
__thread DistributedQueue::State* DistributedQueue::state_;
#else
pthread_key_t DistributedQueue::state_key_;
#endif

void DistributedQueue::Init(TypedAllocator<DistributedQueue::State>* alloc) {
  state_allocator = alloc;
#ifndef HAVE_TLS
  pthread_key_create(&state_key_, NULL);
#endif
}

void DistributedQueue::Init(size_t p) {
  p_ = p;
  for (size_t i = 0; i < p_; i++) {
    backends_[i] = Stack1::New();
  }
}

DistributedQueue::State* DistributedQueue::NewState() {
  State* state = state_allocator->New();
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

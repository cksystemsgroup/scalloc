// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_DISTRIBUTED_QUEUE_H_
#define SCALLOC_DISTRIBUTED_QUEUE_H_

#include <cstdlib>

#include "common.h"
#include "random.h"
#include "page_heap_allocator.h"
#include "spinlock-inl.h"
#include "stack-inl.h"

class DistributedQueue {
 public:
  // Initialize the DQ module in a thread-safe way.
  static void InitModule();

  // Initializer method instead of constructor to enable global use (before
  // main). Note: NOT thread-safe!
  void Init(size_t p);

  void Enqueue(void* p);
  void EnqueueAt(void* p, size_t start);
  void* Dequeue();
  void* DequeueOnlyAt(size_t backend_id);
  void* DequeueAt(size_t start);

 private:
  // Upper bound on DQ backends (and thus also State records).
  static const size_t kMaxBackends = 80;

  typedef Stack Backend;

  struct State {
    uint64_t backend_states[kMaxBackends];
  };


  // Allocator used to get backend stacks.
  static scalloc::PageHeapAllocator<Backend, 64> backend_allocator_;

  // Allocator used to get a State instance.
  static scalloc::PageHeapAllocator<State, 64> state_allocator_;

  // State object used to record backend states by each thread in the emptiness
  // check of a dequeue operation.
  static __thread TLS_MODE State* state_;

  // Number of backends.
  size_t p_;

  // The actual DQ backends.
  Backend* backends_[kMaxBackends] cache_aligned;

  // Create a new threadlocal state.
  State* NewState();
};


always_inline void DistributedQueue::Enqueue(void* p) {
  size_t start = static_cast<size_t>(hwrand()) % p_;
  EnqueueAt(p, start);
}

always_inline void DistributedQueue::EnqueueAt(void* p, size_t start) {
  backends_[start]->Put(p);
}

always_inline void* DistributedQueue::Dequeue() {
  size_t start = static_cast<size_t>(hwrand()) % p_;
  return DequeueAt(start);
}

always_inline void* DistributedQueue::DequeueOnlyAt(size_t backend_id) {
  return backends_[backend_id]->Pop();
}

always_inline void* DistributedQueue::DequeueAt(size_t start) {
  void* result;
  if (state_ == NULL) {
    state_ = NewState();
  }
  size_t i;
  while (true) {
 GET_RETRY:
    for (size_t _cnt = 0; _cnt < p_; _cnt++) {
      i = (_cnt + start) % p_;
      if ((result = backends_[i]->PopRecordState(
              &(state_->backend_states[i]))) != NULL) {
        return result;
      }
    }
    for (size_t _cnt = 0; _cnt < p_; _cnt++) {
      i = (_cnt + start) % p_;
      if (state_->backend_states[i] != backends_[i]->GetState()) {
        start = i;
        goto GET_RETRY;
      }
    }
    return NULL;
  }
}

#endif  // SCALLOC_DISTRIBUTED_QUEUE_H_

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_DISTRIBUTED_QUEUE_H_
#define SCALLOC_DISTRIBUTED_QUEUE_H_

#include <cstdlib>
#include <pthread.h>

#include "common.h"
#include "random.h"
#include "spinlock-inl.h"
#include "stack-inl.h"
#include "typed_allocator.h"

namespace scalloc {

class DistributedQueue {
 public:
  typedef Stack Backend;
  
  // Upper bound on DQ backends (and thus also State records).
  static const size_t kMaxBackends = 80;
  
  struct State {
    uint64_t backend_states[kMaxBackends];
  };

  static void Init(TypedAllocator<State>* sate_alloc,
                   TypedAllocator<Backend>* backend_alloc);

  // Initializer method instead of constructor to enable global use (before
  // main). Note: NOT thread-safe!
  void Init(size_t p);

  void Enqueue(void* p);
  void EnqueueAt(void* p, size_t start);
  void* Dequeue();
  void* DequeueOnlyAt(size_t backend_id);
  void* DequeueAt(size_t start);

 private:
  State* NewState();
  State* GetState();

  static scalloc::TypedAllocator<Backend>* backend_allocator_;
  static scalloc::TypedAllocator<State>* state_allocator_;

#ifdef HAVE_TLS
  // State object used to record backend states by each thread in the emptiness
  // check of a dequeue operation.
  static __thread TLS_MODE State* state_;
#else
  static pthread_key_t state_key_;
#endif  // HAVE_TLS

  // Number of backends.
  size_t p_;

  // The actual DQ backends.
  Backend* backends_[kMaxBackends];
};


inline void DistributedQueue::Enqueue(void* p) {
  size_t start = static_cast<size_t>(hwrand()) % p_;
  EnqueueAt(p, start);
}


inline void DistributedQueue::EnqueueAt(void* p, size_t start) {
  backends_[start]->Put(p);
}


inline void* DistributedQueue::Dequeue() {
  size_t start = static_cast<size_t>(hwrand()) % p_;
  return DequeueAt(start);
}


inline void* DistributedQueue::DequeueOnlyAt(size_t backend_id) {
  return backends_[backend_id]->Pop();
}


inline void* DistributedQueue::DequeueAt(size_t start) {
  void* result;
  State* state = GetState();
  if (state == NULL) {
    state = NewState();
  }
  size_t i;
  while (true) {
 GET_RETRY:
    for (size_t _cnt = 0; _cnt < p_; _cnt++) {
      i = (_cnt + start) % p_;
      if ((result = backends_[i]->PopRecordState(
              &(state->backend_states[i]))) != NULL) {
        return result;
      }
    }
    for (size_t _cnt = 0; _cnt < p_; _cnt++) {
      i = (_cnt + start) % p_;
      if (state->backend_states[i] != backends_[i]->GetState()) {
        start = i;
        goto GET_RETRY;
      }
    }
    return NULL;
  }
}

}  // namespace scalloc

#endif  // SCALLOC_DISTRIBUTED_QUEUE_H_

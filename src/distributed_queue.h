// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_DISTRIBUTED_QUEUE_H_
#define SCALLOC_DISTRIBUTED_QUEUE_H_

#include <cstdlib>

#include "allocators/typed_allocator.h"
#include "common.h"
#include "locked_stack-inl.h"  // Backend: LockedStack
#include "random.h"
#include "stack-inl.h"  // Backend: Stack

#define MAX_BACKENDS 128

#ifndef BACKEND_TYPE
#define BACKEND_TYPE Stack
#endif  // BACKEND_TYPE

namespace scalloc {

class DistributedQueue {
 public:
  typedef BACKEND_TYPE Backend;

  // Upper bound on DQ backends (and thus also State records).
  static const size_t kMaxBackends = MAX_BACKENDS;

  struct State {
    uint64_t backend_states[kMaxBackends];
  };

  static void Init(TypedAllocator<Backend>* backend_alloc);

  inline DistributedQueue() {}
  void Init(size_t p);
  void Enqueue(void* p);
  void EnqueueAt(void* p, const size_t backend_id);
  void EnqueueBufferAt(void* start, void* end, const size_t backend_id);

  void* Dequeue();
  void* DequeueOnlyAt(const size_t backend_id);
  void* DequeueStartAt(const size_t first_backend_id);

 private:
  static scalloc::TypedAllocator<Backend>* backend_allocator_;

  // Number of backends.
  size_t p_;
  size_t p_mask_;

  // The actual DQ backends.
  Backend* backends_[kMaxBackends];

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(DistributedQueue);
};


always_inline void DistributedQueue::Enqueue(void* p) {
  size_t start = static_cast<size_t>(hwrand()) & p_mask_;
  EnqueueAt(p, start);
}


always_inline void DistributedQueue::EnqueueAt(void* p, const size_t backend_id) {
  backends_[backend_id & p_mask_]->Put(p);
}


always_inline void DistributedQueue::EnqueueBufferAt(void* start, void* end, const size_t backend_id) {
  backends_[backend_id & p_mask_]->PushBuffer(start, end);
}


always_inline void* DistributedQueue::Dequeue() {
  size_t start = static_cast<size_t>(hwrand()) & p_mask_;
  return DequeueStartAt(start);
}


always_inline void* DistributedQueue::DequeueOnlyAt(const size_t backend_id) {
  return backends_[backend_id  & p_mask_]->Pop();
}


#ifdef DQ_NON_LIN
always_inline void* DistributedQueue::DequeueStartAt(const size_t first_backend_id) {
  void* result;
  size_t start = first_backend_id & p_mask_;
  size_t i;
  for (size_t _cnt = 0; _cnt < p_; _cnt++) {
    i = (_cnt + start) & p_mask_;
    if ((result = backends_[i]->Pop()) != NULL) {
      return result;
    }
  }
  return NULL;
}
#endif  // DQ_NON_LIN


#ifndef DQ_NON_LIN
always_inline void* DistributedQueue::DequeueStartAt(const size_t first_backend_id) {
  void* result;
  size_t start = first_backend_id & p_mask_;
  State state;
  size_t i;
  while (true) {
 GET_RETRY:
    for (size_t _cnt = 0; _cnt < p_; _cnt++) {
      i = (_cnt + start) & p_mask_;
      if ((result = backends_[i]->PopRecordState(
              &(state.backend_states[i]))) != NULL) {
        return result;
      }
    }
    for (size_t _cnt = 0; _cnt < p_; _cnt++) {
      i = (_cnt + start) & p_mask_;
      if (state.backend_states[i] != backends_[i]->GetState()) {
        start = i;
        goto GET_RETRY;
      }
    }
    return NULL;
  }
}
#endif  // DQ_NON_LIN

}  // namespace scalloc

#endif  // SCALLOC_DISTRIBUTED_QUEUE_H_

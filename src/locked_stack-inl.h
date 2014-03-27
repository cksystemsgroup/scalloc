// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCKED_STACK_INL_H_
#define SCALLOC_LOCKED_STACK_INL_H_

#include "spinlock-inl.h"

class LockedStack {
 public:
  void Init();
  void Push(void* p);
  void* Pop();
  void Put(void* p);
  void* Get();

  // Additional  DistributedQueue interface

  void* PopRecordState(uint64_t* state);
  uint64_t GetState();

 private:
  SpinLock stack_lock_;
  void* top_;
  uint64_t tag_;
};


inline void LockedStack::Init() {
  top_ = NULL;
  tag_ = 0;
  stack_lock_.Reset();
}


inline void LockedStack::Put(void* p) {
  Push(p);
}


inline void* LockedStack::Get() {
  return Pop();
}


inline void LockedStack::Push(void* p) {
  SpinLockHolder a(&stack_lock_);

  *(reinterpret_cast<void**>(p)) = top_;
  top_ = p;
  tag_++;
}


inline void* LockedStack::Pop() {
  SpinLockHolder a(&stack_lock_);

  if (top_ == NULL) {
    return NULL;
  }

  void* p = top_;
  top_ = *(reinterpret_cast<void**>(p));
  tag_++;
  return p;
}


inline void* LockedStack::PopRecordState(uint64_t* state) {
  SpinLockHolder a(&stack_lock_);

  if (top_ == NULL) {
    *state = tag_;
    return NULL;
  }

  void* p = top_;
  top_ = *(reinterpret_cast<void**>(p));
  tag_++;
  return p;
}


inline uint64_t LockedStack::GetState() {
  return tag_;
}

#endif  // SCALLOC_LOCKED_STACK_INL_H_


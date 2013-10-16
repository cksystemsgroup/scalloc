// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_STACK_INL_H_
#define SCALLOC_STACK_INL_H_

#include <stdint.h>
#include <stdio.h>

#include "atomic.h"
#include "common.h"

// Treiber stack
//
// Note: The implementation stores the next pointers in the memory provided, and
// thus needs blocks of at least sizeof(TaggedAtomic) with an alignment of
// AtomicAlign (default 4).
class Stack {
 public:
  static void InitModule() {}

  void Init();
  void Push(void* p);
  void* Pop();

  void Put(void* p);
  void* Get();

  // Additional  DistributedQueue interface

  void* PopRecordState(uint64_t* state);
  uint64_t GetState();

 private:
  TaggedAtomic<void*, uint64_t> top_;
};

inline void Stack::Put(void* p) {
  Push(p);
}

inline void* Stack::Get() {
  return Pop();
}

inline void Stack::Init() {
  top_.Pack(NULL, 0);
}

inline void Stack::Push(void* p) {
  TaggedAtomic<void*, uint64_t> top_old;
  TaggedAtomic<void*, uint64_t> top_new;
  do {
    top_old.CopyFrom(top_);
    // write the old top's pointer into the current block
    *(reinterpret_cast<void**>(p)) = top_old.Atomic();
    top_new.WeakPack(p, top_old.Tag() + 1);
  } while (!top_.AtomicExchange(top_old, top_new));
}

inline void* Stack::Pop() {
  TaggedAtomic<void*, uint64_t> top_old;
  TaggedAtomic<void*, uint64_t> top_new;
  do {
    top_old.CopyFrom(top_);
    // check whether we top points to NULL, which indicates empty
    if (top_old.Atomic() == NULL) {
      return NULL;
    }
    top_new.WeakPack(*(reinterpret_cast<void**>(top_old.Atomic())),
                     top_old.Tag() + 1);
  } while (!top_.AtomicExchange(top_old, top_new));
  return top_old.Atomic();
}

inline void* Stack::PopRecordState(uint64_t* state) {
  TaggedAtomic<void*, uint64_t> top_old;
  TaggedAtomic<void*, uint64_t> top_new;
  do {
    top_old.CopyFrom(top_);
    // check whether we top points to NULL, which indicates empty
    if (top_old.Atomic() == NULL) {
      *state = top_old.Tag();
      return NULL;
    }
    top_new.WeakPack(*(reinterpret_cast<void**>(top_old.Atomic())),
                     top_old.Tag() + 1);
  } while (!top_.AtomicExchange(top_old, top_new));
  return top_old.Atomic();
}

inline uint64_t Stack::GetState() {
  return top_.Tag();
}

#endif  // SCALLOC_STACK_INL_H_

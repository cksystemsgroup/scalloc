// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_STACK_INL_H_
#define SCALLOC_STACK_INL_H_

#include <atomic>
#include <stdint.h>
#include <stdio.h>

#include "atomic.h"
#include "common.h"

// Treiber stack
//
// Note: The implementation stores the next pointers in the memory provided, and
// thus needs blocks of at least sizeof(TaggedValue) with an alignment of
// AtomicAlign (default 4).
class Stack {
 public:
  static void InitModule() {}

  Stack() {}
  void Init();
  void Push(void* p);
  void* Pop();

  void Put(void* p);
  void* Get();

  // Additional  DistributedQueue interface

  void* PopRecordState(uint64_t* state);
  uint64_t GetState();

 private:
  std::atomic<TaggedValue<void*, uint64_t>::RawType> top_;

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(Stack);
};

inline void Stack::Put(void* p) {
  Push(p);
}

inline void* Stack::Get() {
  return Pop();
}

inline void Stack::Init() {
  top_.store(0);
}

inline void Stack::Push(void* p) {
  TaggedValue<void*, uint64_t> top_old;
  TaggedValue<void*, uint64_t> top_new;
  do {
    top_old.raw = top_.load();
    // write the old top's pointer into the current block
    *(reinterpret_cast<void**>(p)) = top_old.Value();
    top_new.Pack(p, top_old.Tag() + 1);
  } while (!top_.compare_exchange_weak(top_old.raw, top_new.raw));
}

inline void* Stack::Pop() {
  TaggedValue<void*, uint64_t> top_old;
  TaggedValue<void*, uint64_t> top_new;
  do {
    top_old.raw = top_.load();
    // check whether we top points to NULL, which indicates empty
    if (top_old.Value() == NULL) {
      return NULL;
    }
    top_new.Pack(*(reinterpret_cast<void**>(top_old.Value())),
                 top_old.Tag() + 1);
  } while (!top_.compare_exchange_weak(top_old.raw, top_new.raw));
  return top_old.Value();
}

inline void* Stack::PopRecordState(uint64_t* state) {
  TaggedValue<void*, uint64_t> top_old;
  TaggedValue<void*, uint64_t> top_new;
  do {
    top_old.raw = top_.load();
    // check whether we top points to NULL, which indicates empty
    if (top_old.Value() == NULL) {
      *state = top_old.raw;
      return NULL;
    }
    top_new.Pack(*(reinterpret_cast<void**>(top_old.Value())),
                 top_old.Tag() + 1);
  } while (!top_.compare_exchange_weak(top_old.raw, top_new.raw));
  return top_old.Value();
}

inline uint64_t Stack::GetState() {
  return top_.load();
}

#endif  // SCALLOC_STACK_INL_H_

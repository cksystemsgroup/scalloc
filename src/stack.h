// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_STACK_H_
#define SCALLOC_STACK_H_

#include <stdint.h>

#include <atomic>

#include "alloc.h"
#include "stack-inl.h"

namespace scalloc {

// scalloc implementation of a Treiber stack.
class Stack1 {
 public:
  static void Init(TypedAllocator<Stack1>* alloc);
  static Stack1* New();

  Stack1();
  void Push(void* p);
  void* Pop();

  void* PopRecordState(uint64_t* state);

  inline uint64_t State() {
    return top_.load();
  }

 private:
  std::atomic<uint64_t> top_;
};

}  // namespace scalloc

#endif  // SCALLOC_STACK_H_

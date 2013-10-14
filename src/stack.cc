// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "stack.h"

#include <cstddef>  // NULL

#include "atomic.h"

namespace {
  const TaggedValue<void*, uint8_t> kNullValue(NULL, 0);

  scalloc::TypedAllocator<scalloc::Stack1>* stack_allocator;
}  // namespace

namespace scalloc {

void Stack1::Init(TypedAllocator<Stack1>* alloc) {
  stack_allocator = alloc;
}

Stack1* Stack1::New() {
  Stack1* s = stack_allocator->New();
  s->top_.store(kNullValue.raw);
  return s;
}

Stack1::Stack1() {
  top_.store(kNullValue.raw);
}

void Stack1::Push(void* p) {
  TaggedValue<void*, uint8_t> top_old(NULL, 0);
  TaggedValue<void*, uint8_t> top_new(NULL, 0);
  do {
    top_old.raw = top_.load();
    *(reinterpret_cast<void**>(p)) = top_old.Value();
    top_new.Pack(p, top_old.Tag() + 1);
  } while (!top_.compare_exchange_weak(top_old.raw, top_new.raw));
}

void* Stack1::PopRecordState(uint64_t* state) {
  TaggedValue<void*, uint8_t> top_old(NULL, 0);
  TaggedValue<void*, uint8_t> top_new(NULL, 0);
  do {
    top_old.raw = top_.load();
    if (top_old.Value() == NULL) {
      *state = top_old.raw;
      return NULL;
    }
    top_new.Pack(*(reinterpret_cast<void**>(top_old.Value())),
                 top_old.Tag() + 1);
  } while (!top_.compare_exchange_weak(top_old.raw, top_new.raw));
  return top_old.Value();
}

void* Stack1::Pop() {
  uint64_t unused;
  return PopRecordState(&unused);
}

}  // namespace scalloc

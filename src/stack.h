// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_STACK_INL_H_
#define SCALLOC_STACK_INL_H_

#include <stdint.h>
#include <stdio.h>

#include <atomic>

#include "atomic_value.h"
#include "globals.h"
#include "log.h"

namespace scalloc {

// Treiber stack
//
// Note: The implementation stores the next pointers in the memory provided, and
// thus needs blocks of at least sizeof(TaggedValue).
template<int PAD = 64>
class Stack {
 public:
  always_inline Stack() : top_(TaggedValue<void*>(nullptr, 0)) { }
  always_inline void Push(void* p);
  always_inline void PushRange(void* p_start, void* p_end);
  always_inline void* Pop();
  always_inline void PopAll(void** elements, int32_t* len);
  always_inline int_fast32_t Length();

  always_inline void SetTop(void* p);

  always_inline bool Empty() {
    return top_.load() == TaggedValue<void*>(NULL, 0);
  }

  always_inline int32_t PushReturnTag(void* p);

 private:
  typedef TaggedValue<void*> TopPtr;
  AtomicTaggedValue<void*> top_;

  int8_t pad_[
    (PAD == 0) ? 0 : PAD - (sizeof(top_) % PAD)];
};


template<int PAD>
void Stack<PAD>::SetTop(void* p) {
  top_.store(TopPtr(p, 0));
}


template<int PAD>
int32_t Stack<PAD>::PushReturnTag(void* p) {
  TopPtr top_old;
  do {
    top_old = top_.load();
    *(reinterpret_cast<void**>(p)) = top_old.value();
  } while (!top_.swap(top_old, TopPtr(p, top_old.tag() + 1)));
  return top_old.tag() + 1;
}


template<int PAD>
void Stack<PAD>::Push(void* p) {
  LOG(kTrace, "push %p", p);
  TopPtr top_old;
  do {
    top_old = top_.load();
    *(reinterpret_cast<void**>(p)) = top_old.value();
  } while (!top_.swap(top_old, TopPtr(p, top_old.tag() + 1)));
}


template<int PAD>
void Stack<PAD>::PushRange(void* p_start, void* p_end) {
  TopPtr top_old;
  do {
    top_old = top_.load();
    *(reinterpret_cast<void**>(p_end)) = top_old.value();
  } while (!top_.swap(top_old, TopPtr(p_start, top_old.tag() + 1)));
}


template<int PAD>
void* Stack<PAD>::Pop() {
  TopPtr top_old;
  do {
    top_old = top_.load();
    if (top_old.value() == NULL) {
      return NULL;
    }
  } while (!top_.swap(
      top_old, TopPtr(*(reinterpret_cast<void**>(top_old.value())),
                      top_old.tag() + 1)));
  LOG(kTrace, "pop  %p", top_old.value());
  return top_old.value();
}


// Returns the length of the list returned iff there have not been any pop()
// operations in between.
template<int PAD>
void Stack<PAD>::PopAll(void** elements, int32_t* len) {
  TopPtr top_old;
  do {
    top_old = top_.load();
    if (top_old.value() == NULL) {
      break;
    }
  } while (!top_.swap(top_old, TopPtr(NULL, 0)));
  *elements = top_old.value();
  *len = top_old.tag();
}


// Returns the length of the list returned iff there have not been any pop()
// operations in between.
template<int PAD>
int_fast32_t Stack<PAD>::Length() {
  return top_.load().tag();
}

}  // namespace scalloc

#endif  // SCALLOC_STACK_INL_H_

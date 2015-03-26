// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_FREE_LIST_H_
#define SCALLOC_FREE_LIST_H_

#include "log.h"
#include "platform/globals.h"
#include "size_classes.h"

namespace scalloc {

class IncrementalFreeList {
 public:
  always_inline IncrementalFreeList(intptr_t start, size_t size_class);
  always_inline int32_t Push(void* obj);
  always_inline void* Pop();
  always_inline void SetList(void* objs, size_t len);

  always_inline int_fast32_t Length() { return len_; }

 private:
  void* list_;              // Incremental free list.
  intptr_t bump_pointer_;
  int32_t len_;        // Number of free objects.
  int32_t increment_;  // Size of an object.
};


IncrementalFreeList::IncrementalFreeList(intptr_t start, size_t size_class)
    : list_(NULL)
    , bump_pointer_(start)
    , len_(ClassToObjects[size_class])
    , increment_(ClassToSize[size_class]) {
}


int32_t IncrementalFreeList::Push(void* obj) {
  *(reinterpret_cast<void**>(obj)) = list_;
  list_ = obj;
  len_++;
  return len_;
}


void IncrementalFreeList::SetList(void* objs, size_t len) {
  list_ = objs;
  len_ = len;
}


void* IncrementalFreeList::Pop() {
  void* result = list_;
  if (result != NULL) {
    list_ = *(reinterpret_cast<void**>(list_));
    len_--;
  } else {
    if (UNLIKELY(len_ == 0)) {
      return NULL;
    }
    result = reinterpret_cast<void*>(bump_pointer_);
    bump_pointer_ += increment_;
    len_--;
  }
  return result;
}

}  // namespace scalloc

#endif  // SCALLOC_FREE_LIST_H_


// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_FREELIST_H_
#define SCALLOC_FREELIST_H_

#include "scalloc_assert.h"
#include "common.h"
#include "size_classes.h"

namespace scalloc {

/// An unlocked free list.
class Freelist {
 public:
  inline Freelist() {}
  void Init(const void* start, const size_t size_class);

  void Push(void* p);
  void* Pop();
  size_t Utilization();
  inline bool Empty() { return len_ == 0; }
  inline bool Full() { return len_ == cap_; }
  inline size_t Size() { return len_; }

 private:
  size_t cap_;
  size_t len_;
  void* list_;

  size_t size_class_;
  void* bp_;

#ifdef DEBUG
  uintptr_t lower_;
  uintptr_t upper_;
#endif  // DEBUG

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(Freelist);
};


inline void Freelist::Init(const void* start,
                           const size_t size_class) {
  size_class_ = size_class;
  len_ = ClassToObjects[size_class];
  cap_ = ClassToObjects[size_class];
  list_ = NULL;
#ifdef DEBUG
  const size_t size = ClassToSize[size_class];
  uintptr_t start_ptr = reinterpret_cast<uintptr_t>(start);
  lower_ = start_ptr;
  upper_ = start_ptr + size * ClassToObjects[size_class];
#endif  // DEBUG
  bp_ = (void*)start;
  /*
  for (size_t len = len_; len > 0; len--) {
    // Inlined Freelist::Push.
    *(reinterpret_cast<void**>(start_ptr)) = list_;
    list_ = reinterpret_cast<void*>(start_ptr);
    start_ptr += size;
  }
  */
}


inline void Freelist::Push(void* p) {
  ScallocAssert((reinterpret_cast<uintptr_t>(p) >= lower_) &&
                (reinterpret_cast<uintptr_t>(p) < upper_));
  *(reinterpret_cast<void**>(p)) = list_;
  list_ = p;
  len_++;
}


inline void* Freelist::Pop() {
  void* result = list_;
  if (result != NULL) {
    ScallocAssert((reinterpret_cast<uintptr_t>(result) >= lower_) &&
                  (reinterpret_cast<uintptr_t>(result) < upper_));
    list_ = *(reinterpret_cast<void**>(list_));
    len_--;
  } else {
    if (len_ == 0) {
      return NULL;
    }
    result = bp_;
    bp_ = (void*)(reinterpret_cast<uintptr_t>(bp_) + ClassToSize[size_class_]);
    len_--;
  }
  return result;
}


inline size_t Freelist::Utilization() {
  return 100 - ((len_ * 100) / cap_);
}

}  // namespace scalloc

#endif  // SCALLOC_FREELIST_H_

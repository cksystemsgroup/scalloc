// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_FREELIST_H_
#define SCALLOC_FREELIST_H_

#include "common.h"
#include "scalloc_assert.h"
#include "size_classes.h"

namespace scalloc {

class FreelistBase {
 public:
  virtual void Init(const void* start, const size_t size_class) = 0;
  virtual void Push(void* p) = 0;
  virtual void* Pop() = 0;

  inline bool Empty() { return len_ == 0; }
  inline bool Full() { return len_ == cap_; }
  inline size_t Size() { return len_; }
  inline size_t Utilization() { return 100 - ((len_ * 100) / cap_); }

 protected:
  size_t cap_;
  size_t len_;

#ifdef DEBUG
  uintptr_t lower_;
  uintptr_t upper_;
#endif  // DEBUG
};


class IncrementalFreelist : public FreelistBase {
 public:
  inline void Init(const void* start, const size_t size_class) {
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
    bp_ = const_cast<void*>(start);
  }

  inline void* Pop() {
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
      ScallocAssert((reinterpret_cast<uintptr_t>(result) >= lower_) &&
                    (reinterpret_cast<uintptr_t>(result) < upper_));
      bp_ = reinterpret_cast<void*>(
          reinterpret_cast<uintptr_t>(bp_) + ClassToSize[size_class_]);
      len_--;
    }
    return result;
  }

  inline void Push(void* p) {
    ScallocAssert((reinterpret_cast<uintptr_t>(p) >= lower_) &&
                  (reinterpret_cast<uintptr_t>(p) < upper_));
    *(reinterpret_cast<void**>(p)) = list_;
    list_ = p;
    len_++;
  }

 protected:
  void* list_;
  void* bp_;
  size_t size_class_;
};


class BatchedFreelist : public FreelistBase {
 public:
  inline void Init(const void* start, const size_t size_class) {
    len_ = ClassToObjects[size_class];
    cap_ = ClassToObjects[size_class];
    list_ = NULL;
    const size_t size = ClassToSize[size_class];
    uintptr_t start_ptr = reinterpret_cast<uintptr_t>(start);
#ifdef DEBUG
    lower_ = start_ptr;
    upper_ = start_ptr + size * ClassToObjects[size_class];
#endif  // DEBUG
    for (size_t len = len_; len > 0; len--) {
      // Inlined Freelist::Push.
      *(reinterpret_cast<void**>(start_ptr)) = list_;
      list_ = reinterpret_cast<void*>(start_ptr);
      start_ptr += size;
    }
  }

  inline void Push(void* p) {
    ScallocAssert((reinterpret_cast<uintptr_t>(p) >= lower_) &&
                  (reinterpret_cast<uintptr_t>(p) < upper_));
    *(reinterpret_cast<void**>(p)) = list_;
    list_ = p;
    len_++;
  }

  inline void* Pop() {
    void* result = list_;
    if (result != NULL) {
      ScallocAssert((reinterpret_cast<uintptr_t>(result) >= lower_) &&
                    (reinterpret_cast<uintptr_t>(result) < upper_));
      list_ = *(reinterpret_cast<void**>(list_));
      len_--;
    }
    return result;
  }

 protected:
  void* list_;
};

}  // namespace scalloc

#endif  // SCALLOC_FREELIST_H_

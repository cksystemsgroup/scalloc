// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_FREELIST_H_
#define SCALLOC_FREELIST_H_

#include "common.h"
#include "platform/assert.h"
#include "size_classes.h"

namespace scalloc {

class FreelistBase {
 public:
  virtual void Init(const void* start, const size_t size_class) = 0;
  virtual void Push(void* p) = 0;
  virtual void* Pop() = 0;

  // Returns true if the free list is empty.
  always_inline bool Empty() { return len_ == 0; }

  // Returns true if the free list if full.
  always_inline bool Full() { return len_ == cap_; }

  // Returns the current number of free objects in this free list.
  always_inline size_t Size() { return len_; }

  // The utilization in percent (%) of this free list. For zero allocated
  // objects the utilization is 0%.
  always_inline size_t Utilization() { return 100 - ((len_ * 100) / cap_); }

 protected:
  // The capacity, i.e., the overall number of objects of this free list.
  size_t cap_;
  
  // The current length of the free list, i.e., the number of free objects in
  // this free list.
  size_t len_;

  uintptr_t upper_;
#ifdef DEBUG
  uintptr_t lower_;
#endif  // DEBUG
};


class IncrementalFreelist : public FreelistBase {
 public:
  typedef void* (*pop_fcn)(IncrementalFreelist* thiz);

  inline void Init(const void* start, const size_t size_class) {
    size_class_ = size_class;
    len_ = ClassToObjects[size_class];
    cap_ = ClassToObjects[size_class];
    increment_ = ClassToSize[size_class];
    list_ = NULL;
    const size_t size = ClassToSize[size_class];
    uintptr_t start_ptr = reinterpret_cast<uintptr_t>(start);
    upper_ = start_ptr + size * ClassToObjects[size_class];
#ifdef DEBUG
    lower_ = start_ptr;
#endif  // DEBUG
    bp_ = const_cast<void*>(start);
    done_ = false;
    pop_ = PopBp;
    bp_len_ = cap_;
  }

  always_inline void* Pop() {
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
          reinterpret_cast<uintptr_t>(bp_) + increment_);
      len_--;
    }
    return result;
  }

  static void* PopList(IncrementalFreelist* thiz) {
    void* result = thiz->list_;
    if (result != NULL) {
      thiz->list_ = *(reinterpret_cast<void**>(thiz->list_));
      thiz->len_--;
    }
    return result;
  }

  static void* PopBp(IncrementalFreelist* thiz) {
    void* result = thiz->bp_;
    thiz->bp_ = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(thiz->bp_) + thiz->increment_);
    thiz->bp_len_--;
    thiz->len_--;
    if (thiz->bp_len_ == 0) {
      thiz->pop_ = PopList;
    }
    return result;
  }

  always_inline void Push(void* p) {
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
  size_t increment_;
  bool done_; 
  pop_fcn pop_;
  size_t bp_len_;
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

  always_inline void Push(void* p) {
    ScallocAssert((reinterpret_cast<uintptr_t>(p) >= lower_) &&
                  (reinterpret_cast<uintptr_t>(p) < upper_));
    *(reinterpret_cast<void**>(p)) = list_;
    list_ = p;
    len_++;
  }

  always_inline void* Pop() {
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

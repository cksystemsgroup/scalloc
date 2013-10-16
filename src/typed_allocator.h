// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_TYPED_ALLOCATOR_H_
#define SCALLOC_TYPED_ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "common.h"
#include "scalloc_arenas.h"
#include "spinlock-inl.h"
#include "stack-inl.h"
#include "utils.h"

namespace scalloc {

// Almost lock-free free-list allocator that may be used internally for fixed
// types and alignments.
template<typename T>
class TypedAllocator {
 public:
  static const size_t kNoAlignment = 1;

  // No constructor, but an init function, because TypedAllocator must be
  // available from a global context (before main).
  //
  // alloc_increment_ needs to be a multiple of the system page size.
  void Init(size_t alloc_increment, size_t alignment) {
    alloc_increment_ = alloc_increment;

    tsize_ = sizeof(T);
    if (alignment > kNoAlignment) {
      if (tsize_ + alignment <= tsize_) {
        Fatal("PageHeapAllocator: overflow");
      }
      if (kPageSize % alignment != 0) {
        Fatal("PageHeapAllocator: ALIGNMENT must be a divisor of system "
              "page size");
      }
      tsize_ = utils::PadSize(tsize_, alignment);
    }

    if (tsize_ > alloc_increment_) {
      Fatal("PageHeapAllocator: type T is too large for current "
            "allocation increment.");
    }

    free_list_.Init();
  }

  void* Refill() {
    void* result = reinterpret_cast<void*>(
        InternalArena.Allocate(alloc_increment_));
    uintptr_t ptr = reinterpret_cast<uintptr_t>(result) + tsize_;
    while(ptr < (reinterpret_cast<uintptr_t>(result) + alloc_increment_)) {
      free_list_.Push(reinterpret_cast<void*>(ptr));
      ptr += tsize_;
    }
    return result;
  }

  T* New() {
    void* result = free_list_.Pop();

    if (result == NULL) {
      LockScope(refill_lock_);

      result = free_list_.Pop();
      if (result == NULL) {
        result = Refill();
      }
    }

    return reinterpret_cast<T*>(result);
  }

  void Delete(T* ptr) {
    free_list_.Push(ptr);
  }

 private:
  size_t alloc_increment_;
  size_t tsize_;
  SpinLock refill_lock_;
  Stack free_list_;
};

}  // namespace scalloc

#endif  // SCALLOC_TYPED_ALLOCATOR_H_

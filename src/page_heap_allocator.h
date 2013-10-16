// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PAGE_HEAP_ALLOCATOR_
#define SCALLOC_PAGE_HEAP_ALLOCATOR_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "scalloc_arenas.h"
#include "spinlock-inl.h"
#include "stack-inl.h"

namespace scalloc {

// Since an alignment of 1 does not make any sense we can use it to indicate no
// alignment.
const int kNoAlignment = 1;

// Almost lock-free free-list allocator that may be used internally for fixed
// types and alignments.
template<typename T, int ALIGNMENT = kNoAlignment>
class PageHeapAllocator {
 public:
  // No constructor, but an init function, because PageHeapAllocator must be
  // available from a global context (before main).
  //
  // alloc_increment_ needs to be a multiple of the system page size.
  void Init(size_t alloc_increment) {
    alloc_increment_ = alloc_increment;

    tsize_ = sizeof(T);
    if (ALIGNMENT > kNoAlignment) {
      if (tsize_ + ALIGNMENT <= tsize_) {
        Fatal("PageHeapAllocator: overflow");
      }
      if (kPageSize % ALIGNMENT != 0) {
        Fatal("PageHeapAllocator: ALIGNMENT must be a divisor of system "
              "page size");
      }
      tsize_ = PadSize(tsize_, ALIGNMENT);
    }

    if (tsize_ > alloc_increment_) {
      Fatal("PageHeapAllocator: type T is too large for current "
            "allocation increment.");
    }

    free_list_.Init();
  }

  void* Refill() {
    uintptr_t ptr = reinterpret_cast<uintptr_t>(
        InternalArena.Allocate(alloc_increment_));
    void* result = reinterpret_cast<void*>(ptr);
    ptr += tsize_;
    for (size_t i = 1; i < alloc_increment_/tsize_; i++) {
      free_list_.Push(reinterpret_cast<void*>(ptr));
      ptr += tsize_;
    }
    return result;
  }

  T* New() {
    void* result = free_list_.Pop();
    if (result == NULL) {
      SpinLockHolder lock_holder(&refill_lock_);
      CompilerBarrier();
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

#endif  // SCALLOC_PAGE_HEAP_ALLOCATOR_

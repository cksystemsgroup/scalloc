// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PAGE_HEAP_ALLOCATOR_
#define SCALLOC_PAGE_HEAP_ALLOCATOR_

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "system-alloc.h"

namespace scalloc {

// Since an alignment of 1 does not make any sense we can use it to indicate no
// alignment.
const int kNoAlignment = 1;

// Simple bump pointer allocator for objects of a given type T.  External
// locking required.
template<typename T, int ALIGNMENT = kNoAlignment>
class PageHeapAllocator {
 public:
  // No constructor, but an init function, because PageHeapAllocator must be
  // available from a global context (before main).
  //
  // alloc_increment_ needs to be a multiple of kSystemPageSize.
  void Init(size_t alloc_increment) {
    alloc_increment_ = alloc_increment;

    tsize_ = sizeof(T);
    if (ALIGNMENT > kNoAlignment) {
      if (tsize_ + ALIGNMENT <= tsize_) {
        ErrorOut("PageHeapAllocator: overflow");
      }
      if (kSystemPageSize % ALIGNMENT != 0) {
        ErrorOut("PageHeapAllocator: ALIGNMENT must be a divisor of system "
                 "page size");
      }
      tsize_ = PadSize(tsize_, ALIGNMENT);
    }

    if (tsize_ > alloc_increment_) {
      ErrorOut("PageHeapAllocator: type T is too largefor current "
               "allocation increment.");
    }

    free_area_ = NULL;
    free_available_ = 0;
    free_list_ = NULL;
  }

  T* New() {
    void* result;
    if (free_list_ != NULL) {
      result = free_list_;
      free_list_ = *(reinterpret_cast<void**>(result));
    } else {
      if (free_available_ < tsize_) {
        free_area_ = reinterpret_cast<char*>(SystemAlloc_Mmap(
            alloc_increment_, NULL));
        if (free_area_ == NULL) {
          ErrorOut("PageHeapAllocator: out of memory");
        }
        free_available_ = alloc_increment_;
      }
      // Bump pointer
      result = free_area_;
      free_area_ += tsize_;
      free_available_ -= tsize_;
    }
    return reinterpret_cast<T*>(result);
  }

  void Delete(T* ptr) {
    *(reinterpret_cast<void**>(ptr)) = free_list_;
    free_list_ = ptr;
  }

 private:
  size_t alloc_increment_;
  size_t tsize_;
  size_t free_available_;
  char* free_area_;
  void* free_list_;
};

}  // namespace scalloc

#endif  // SCALLOC_PAGE_HEAP_ALLOCATOR_

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_LARGE_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_LARGE_ALLOCATOR_H_

#include <cstddef>

#include "assert.h"
#include "common.h"
#include "headers.h"
#include "utils.h"

namespace scalloc {

class LargeAllocator {
 public:
  static void* Alloc(size_t size);
  static void Free(LargeObjectHeader* lbh);
  static bool Owns(const void* p);

 private:
  DISALLOW_ALLOCATION();
  DISALLOW_IMPLICIT_CONSTRUCTORS(LargeAllocator);
};


inline void* LargeAllocator::Alloc(size_t size) {
  LOG_CAT("large-alloc", kTrace, "request size: %lu", size);
  size = utils::PadSize(size + sizeof(LargeObjectHeader), kPageSize);
  size_t actual_size;
  uintptr_t p = reinterpret_cast<uintptr_t>(
      scalloc::utils::SystemMmap(size, &actual_size, false));
  if (UNLIKELY(p % kPageSize != 0)) {
    Fatal("large malloc alignment failed");
  }
  LargeObjectHeader* lbh = reinterpret_cast<LargeObjectHeader*>(p);
  lbh->Reset(actual_size);
  LOG_CAT("large-alloc", kTrace, "allocation size: %lu ,p: %p",
          size, reinterpret_cast<void*>(p));
  return reinterpret_cast<void*>(p + sizeof(*lbh));
}


inline void LargeAllocator::Free(LargeObjectHeader* lbh) {
  LOG_CAT("large-alloc", kTrace, "free: %p size: %lu",
          reinterpret_cast<void*>(lbh), lbh->size);
  scalloc::utils::SystemMunmap(reinterpret_cast<void*>(lbh), lbh->size);
}


inline bool LargeAllocator::Owns(const void* p) {
  LargeObjectHeader* lbh = reinterpret_cast<LargeObjectHeader*>(
      const_cast<void*>(p));
  return lbh->type == kLargeObject;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_LARGE_ALLOCATOR_H_

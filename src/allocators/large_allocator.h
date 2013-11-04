// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_LARGE_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_LARGE_ALLOCATOR_H_

#include <cstddef>

#include "block_header.h"
#include "common.h"
#include "system-alloc.h"
#include "utils.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

class LargeAllocator {
 public:
  static void* Alloc(size_t size);
  static void Free(LargeObjectHeader* lbh);
  static bool Owns(const void* p);
};


inline void* LargeAllocator::Alloc(size_t size) {
  LOG(kTrace, "[LargeAllocator] request size: %lu", size);
  size = utils::PadSize(size + sizeof(LargeObjectHeader), kPageSize);
  size_t actual_size;
  uintptr_t p = reinterpret_cast<uintptr_t>(
      scalloc::SystemAlloc_Mmap(size, &actual_size));
  if (UNLIKELY(p % kPageSize != 0)) {
    Fatal("large malloc alignment failed");
  }
  LargeObjectHeader* lbh = reinterpret_cast<LargeObjectHeader*>(p);
  lbh->Reset(actual_size);
#ifdef PROFILER_ON
  Profiler::GetProfiler().LogAllocation(size);
#endif  // PROFILER_ON
  LOG(kTrace, "[LargeAllocator] allocation size: %lu ,p: %p",
      size, reinterpret_cast<void*>(p));
  return reinterpret_cast<void*>(p + sizeof(*lbh));
};


inline void LargeAllocator::Free(LargeObjectHeader* lbh) {
#ifdef PROFILER_ON
  Profiler::GetProfiler().LogDeallocation(lbh->size);
#endif  // PROFILER_ON
  LOG(kTrace, "[LargeAllocator] free: %p size: %lu",
      reinterpret_cast<void*>(lbh), lbh->size);
  scalloc::SystemFree_Mmap(reinterpret_cast<void*>(lbh), lbh->size);
}


inline bool LargeAllocator::Owns(const void* p) {
  LargeObjectHeader* lbh = reinterpret_cast<LargeObjectHeader*>(
      const_cast<void*>(p));
  return lbh->type == kLargeObject;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_LARGE_ALLOCATOR_H_

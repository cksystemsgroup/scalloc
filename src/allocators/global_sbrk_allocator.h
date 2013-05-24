// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_

#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>  // madvise

#include "common.h"
#include "log.h"
#include "runtime_vars.h"

class GlobalSbrkAllocator {
 public:
  void Init(size_t size);
  void* Allocate(const size_t size);
  void Free(void* p, size_t len);

 private:
  uintptr_t current_;
  size_t size_;
} cache_aligned;

inline void* GlobalSbrkAllocator::Allocate(const size_t size) {
  return reinterpret_cast<void*>(__sync_fetch_and_add(&current_, size));
}

inline void GlobalSbrkAllocator::Free(void* p, size_t len) {
  if (reinterpret_cast<uintptr_t>(p) > current_) {
    ErrorOut("[GlobalSbrkAllocator]: cannot free space");
  }
  if (madvise(p, len, MADV_DONTNEED) == -1) {
    ErrorOut("[GlobalSbrkAllocator]: madvise failed. errno: %lu", errno);
  }
}

#endif  // SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_

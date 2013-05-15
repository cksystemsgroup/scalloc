// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_

#include <stdint.h>

#include "common.h"
#include "runtime_vars.h"

class GlobalSbrkAllocator {
 public:
  static void InitModule();
  static void* Allocate(const size_t size);

 private:
  static cache_aligned uintptr_t current_;
};

inline void* GlobalSbrkAllocator::Allocate(const size_t size) {
  return reinterpret_cast<void*>(__sync_fetch_and_add(&current_, size));
}

#endif  // SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_

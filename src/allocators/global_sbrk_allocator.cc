// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/global_sbrk_allocator.h"

#include <errno.h>
#include <sys/mman.h>  // mmap

#include "common.h"
#include "log.h"

#ifdef SBRK_SPACE
const size_t kMmapSize = SBRK_SPACE;
#else  // SBRK_SPACE

#ifdef __x86_64__
const size_t kMmapSize = 1UL << 35;  // 32GiB
#endif

#ifdef __i386__
const size_t kMmapSize = 1UL << 31;  // 2GiB
#endif

#endif

uintptr_t GlobalSbrkAllocator::current_;

void GlobalSbrkAllocator::InitModule() {
  void* p = mmap(0, kMmapSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED) {
    ErrorOut("[GlobalSbrkAllocator] mmap failed. errno: %lu", errno);
  }
  current_ = reinterpret_cast<uintptr_t>(p);
}

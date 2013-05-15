// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/global_sbrk_allocator.h"

#include <errno.h>
#include <sys/mman.h>  // mmap

#include "log.h"

uintptr_t GlobalSbrkAllocator::current_;

void GlobalSbrkAllocator::InitModule() {
#ifdef __x86_64__
  size_t size = 1UL << 35;  // 32GiB
#endif
#ifdef __i386__
  size_t size = 1UL << 31;
#endif

  void* p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED) {
    ErrorOut("[GlobalSbrkAllocator] mmap failed. errno: %lu", errno);
  }
  current_ = reinterpret_cast<uintptr_t>(p);
}

// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "system-alloc.h"

#include <stdint.h>  // intptr_t and friends
#include <sys/mman.h>  // mmap

#include "assert.h"
#include "common.h"
#include "utils.h"

namespace scalloc {

void* SystemAlloc_Mmap(size_t size, size_t* actual_size) {
  // pad size to some multiple of the system page size
  size = utils::PadSize(size, kPageSize);

  if (actual_size) {
    *actual_size = size;
  }

  void* p = mmap(
      0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if ((reinterpret_cast<uintptr_t>(p) % kPageSize) != 0) {
    Fatal("mmap() did not returned system page size aligned memory");
  }
  return p;
}

void SystemFree_Mmap(void* p, const size_t actual_size) {
  if (munmap(p, actual_size) != 0) {
    Fatal("munmap() failed");
  }
}

}  // namespace scalloc

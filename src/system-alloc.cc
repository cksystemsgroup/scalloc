// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "system-alloc.h"

#include <errno.h>
#include <stdint.h>  // intptr_t and friends
#include <sys/mman.h>  // mmap

#include "scalloc_assert.h"
#include "platform.h"
#include "utils.h"

namespace scalloc {

void* SystemAlloc_Mmap(size_t size, size_t* actual_size, bool huge = false) {
  // pad size to some multiple of the system page size
  size = utils::PadSize(size, kPageSize);

  if (actual_size) {
    *actual_size = size;
  }

  const int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  if (huge) {
    flags |= MAP_HUGETLB;
  }
  /*
#ifdef HUGE_PAGE
  flags |= MAP_HUGETLB;
#endif  // HUGE_PAGE
*/

  void* p = mmap(0, size, prot, flags, -1, 0);
  if (reinterpret_cast<void*>(p) == MAP_FAILED) {
    Fatal("mmap failed. size: %lu, errno: %lu\n", size, errno);
  }
  ScallocAssert(reinterpret_cast<uintptr_t>(p) % kPageSize == 0);
  return p;
}


void SystemFree_Mmap(void* p, const size_t actual_size) {
  if (munmap(p, actual_size) != 0) {
    if (reinterpret_cast<void*>(p) == MAP_FAILED) {
      Fatal("munmap failed. p: %p, errno: %lu\n", p, errno);
    }
  }
}

}  // namespace scalloc

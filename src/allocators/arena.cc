// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/arena.h"

#include "system-alloc.h"

namespace scalloc {

void Arena::Init(size_t size) {
  size_ = size;
  size = size * 2 - kPageSize;
  uintptr_t p = reinterpret_cast<uintptr_t>(SystemAlloc_Mmap(size, NULL));
  p += size_ - (p % size_);
  start_ = p;
  cur_.store(p);
}

}  // namespace scalloc

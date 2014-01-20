// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/arena.h"

#include "system-alloc.h"

namespace scalloc {

void Arena::Init(const size_t size) {
  // Allocates an arena of size bytes also aligning the arena to size.
  size_ = size;
  start_ = reinterpret_cast<uintptr_t>(SystemAlloc_Mmap(
               2 * size_ - kPageSize, NULL));
  // Align the start address.
  start_ += size_ - (start_ % size_);
  cur_.store(start_);
}

}  // namespace scalloc

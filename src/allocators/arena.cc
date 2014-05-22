// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/arena.h"

#include "common.h"
#include "utils.h"

namespace scalloc {

void Arena::Init(const size_t size) {
  // Allocates an arena of size bytes also aligning the arena to size.
  size_ = size;
  // Allocate and align start_ to kVirtualSpanSize.
  bool huge = false;
#ifdef HUGE_PAGE
  huge = true;
#endif
  start_ = reinterpret_cast<uintptr_t>(utils::SystemMmap(
               size_ + kVirtualSpanSize, NULL, huge));
  // Align the start address.
  start_ += kVirtualSpanSize - (start_ % kVirtualSpanSize);
  cur_.store(start_);
}

}  // namespace scalloc

// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SYSTEM_ALLOC_H_
#define SCALLOC_SYSTEM_ALLOC_H_

#include <stdlib.h>

namespace scalloc {

// Allocates a memory block of a given size using mmap() and returns the block's
// address.
//
// Also stores the actual allocation size in actual_size, if a non-NULL pointer
// is provided.
void* SystemAlloc_Mmap(size_t size, size_t* actual_size);

// Frees a memory block that has previously been allocated with
// SystemAlloc_Mmap().
void  SystemFree_Mmap(void* p, const size_t actual_size);

}  // namespace scalloc

#endif  // SCALLOC_SYSTEM_ALLOC_H_

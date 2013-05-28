// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ARENA_H_
#define SCALLOC_ARENA_H_

#include "allocators/global_sbrk_allocator.h"

extern GlobalSbrkAllocator SmallArena;

void InitArenas();

#endif  // SCALLOC_ARENA_H_

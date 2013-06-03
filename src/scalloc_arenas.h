// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ARENA_H_
#define SCALLOC_ARENA_H_

#include "allocators/arena.h"

namespace scalloc {

extern Arena SmallArena;
extern Arena MediumArena;
extern Arena InternalArena;

void InitArenas();

}  // namespace scalloc

#endif  // SCALLOC_ARENA_H_
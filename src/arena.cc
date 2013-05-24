// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "arena.h"

#include "common.h"

cache_aligned GlobalSbrkAllocator SmallArena;
cache_aligned GlobalSbrkAllocator MediumArena;

void InitArenas() {
  SmallArena.Init(kSmallSpace);
  MediumArena.Init(kSmallSpace);
}


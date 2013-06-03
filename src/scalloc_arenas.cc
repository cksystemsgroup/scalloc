// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "scalloc_arenas.h"

#include "common.h"

namespace scalloc {

cache_aligned Arena InternalArena;
cache_aligned Arena SmallArena;
cache_aligned Arena MediumArena;

void InitArenas() {
  InternalArena.Init(kSmallSpace);
  SmallArena.Init(kSmallSpace);
  MediumArena.Init(kSmallSpace);
}

}  // namespace scalloc

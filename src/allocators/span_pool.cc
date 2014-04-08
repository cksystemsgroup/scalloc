// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/span_pool.h"

#include "common.h"
#include "log.h"
#include "scalloc_arenas.h"
#include "utils.h"

namespace scalloc {

cache_aligned SpanPool SpanPool::span_pool_;


void SpanPool::Init() {
  for (unsigned i = 0; i < kNumClasses; ++i) {
    span_pool_.size_class_pool_[i].Init(utils::Parallelism());
  }
}


void* SpanPool::RefillOne() {
  return SmallArena.Allocate(kVirtualSpanSize);
}

}  // namespace scalloc

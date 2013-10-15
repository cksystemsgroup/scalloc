// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/span_pool.h"

#include "scalloc_arenas.h"
#include "utils.h"  // Cpus()

namespace scalloc {

SpanPool SpanPool::span_pool_ cache_aligned;

void SpanPool::Init() {
  unsigned num_cores = utils::Cpus();
  for (unsigned i = 0; i < kNumClasses; ++i) {
    span_pool_.size_class_pool_[i].Init(num_cores);
  }
}

void* SpanPool::RefillOne() {
  return reinterpret_cast<void*>(SmallArena.Allocate(kVirtualSpanSize));
}

}  // namespace scalloc

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "allocators/span_pool.h"
#include "distributed_queue.h"
#include "runtime_vars.h"
#include "scalloc_arenas.h"

namespace {

void InitHeap() {
  RuntimeVars::InitModule();
  scalloc::InitArenas();

  DistributedQueue::InitModule();
  scalloc::SpanPool::InitModule();
}

}  // namespace

TEST(SpanPool, Get) {
  InitHeap();
  scalloc::SpanPool& p = scalloc::SpanPool::Instance();
  void* mem = p.Get();
  EXPECT_TRUE(mem != NULL);
}

TEST(SpanPool, PutNoSegfault) {
  InitHeap();
  scalloc::SpanPool& p = scalloc::SpanPool::Instance();
  void* mem = p.Get();
  p.Put(mem);
}

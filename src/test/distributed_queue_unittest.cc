// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "allocators/arena.h"
#include "common.h"
#include "distributed_queue.h"
#include "runtime_vars.h"
#include "scalloc_arenas.h"

namespace {

const size_t kBackends = 80;

void* GetObject() {
  void* mem = NULL;
  if (posix_memalign(&mem, 16, 16) != 0) {
    fprintf(stderr, "posix_memalign failed\n");
    abort();
  }
  return mem;
}

DistributedQueue* InitDQ() {
  RuntimeVars::InitModule();
  scalloc::InitArenas();
  DistributedQueue::InitModule();

  static DistributedQueue dq;
  dq.Init(kBackends);
  return &dq;
}

}  // namespace

TEST(DistributedQueue, Features) {
  // we cannot init the arean multiple times
  // so we put all tests together in one
  DistributedQueue* dq = InitDQ();
  
  // Init
  EXPECT_TRUE(dq != NULL);
  // InitEmpty
  EXPECT_TRUE(dq->Dequeue() == NULL);  
  
  // EnqueueDequeue
  void* mem = GetObject();
  dq->Enqueue(mem);
  EXPECT_EQ(dq->Dequeue(), mem);
  EXPECT_TRUE(dq->Dequeue() == NULL);

  // DequeueOnlyAt
  mem = GetObject();
  dq->EnqueueAt(mem, 0);
  EXPECT_TRUE(dq->DequeueOnlyAt(1) == NULL);
  EXPECT_EQ(dq->DequeueOnlyAt(0), mem);
  EXPECT_TRUE(dq->DequeueOnlyAt(0) == NULL);

}


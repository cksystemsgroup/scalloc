// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "distributed_queue.h"

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
  DistributedQueue::InitModule();
  static DistributedQueue dq;
  dq.Init(kBackends);
  return &dq;
}

}  // namespace

TEST(DistributedQueue, Init) {
  DistributedQueue* dq = InitDQ();
  EXPECT_TRUE(dq != NULL);
}

TEST(DistributedQueue, InitEmpty) {
  DistributedQueue* dq = InitDQ();
  EXPECT_TRUE(dq->Dequeue() == NULL);
}

TEST(DistributedQueue, EnqueueDequeue) {
  DistributedQueue* dq = InitDQ();
  void* mem = GetObject();
  dq->Enqueue(mem);
  EXPECT_EQ(dq->Dequeue(), mem);
  EXPECT_TRUE(dq->Dequeue() == NULL);
}

TEST(DistributedQueue, DequeueOnlyAt) {
  DistributedQueue* dq = InitDQ();
  void* mem = GetObject();
  dq->EnqueueAt(mem, 0);
  EXPECT_TRUE(dq->DequeueOnlyAt(1) == NULL);
  EXPECT_EQ(dq->DequeueOnlyAt(0), mem);
  EXPECT_TRUE(dq->DequeueOnlyAt(0) == NULL);
}

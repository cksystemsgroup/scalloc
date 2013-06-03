// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <string.h>

#include "allocators/small_allocator.h"
#include "allocators/span_pool.h"
#include "distributed_queue.h"
#include "runtime_vars.h"
#include "scalloc_arenas.h"

namespace {
}  // namespace


class SmallAllocatorTest : public testing::Test {
 protected:
  virtual void SetUp() {
    // global initialization
    RuntimeVars::InitModule();
    scalloc::SizeMap::Instance().Init();

    scalloc::InitArenas();

    DistributedQueue::InitModule();
    scalloc::SpanPool::InitModule();
    scalloc::SmallAllocator::InitModule();

    // Let's play the linker and initialize memory to 0.
    memset(&allocator, 0, sizeof(scalloc::SmallAllocator));

    // Init THIS particular slab size class allocator
    allocator.Init(0);
  }

  scalloc::SmallAllocator allocator;
};

TEST_F(SmallAllocatorTest, SizeZero) {
  EXPECT_TRUE(allocator.Allocate(0) == NULL);
}

TEST_F(SmallAllocatorTest, NonNullMemory) {
  void* memory = allocator.Allocate(sizeof(uint64_t));
  EXPECT_TRUE(memory != NULL);
  BlockHeader* hdr = SpanHeader::GetFromObject(memory);
  EXPECT_EQ(hdr->type, kSlab);
  allocator.Free(memory, reinterpret_cast<SlabHeader*>(hdr));
}

TEST_F(SmallAllocatorTest, Locality) {
  void* p1 = allocator.Allocate(sizeof(uint64_t));
  EXPECT_TRUE(p1 != NULL);
  BlockHeader* hdr = BlockHeader::GetFromObject(p1);
  EXPECT_EQ(hdr->type, kSlab);
  allocator.Free(p1, reinterpret_cast<SlabHeader*>(hdr));
  void* p2 = allocator.Allocate(sizeof(uint64_t));
  EXPECT_EQ(p1, p2);
}

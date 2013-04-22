// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "allocators/large_object_allocator.h"

TEST(LargeObjectAllocator, Alloc) {
  void* mem = scalloc::LargeObjectAllocator::Alloc(512);
  EXPECT_TRUE(mem != NULL);
}

TEST(LargeObjectAllocator, AllocFree) {
  void* mem = scalloc::LargeObjectAllocator::Alloc(512);
  EXPECT_TRUE(mem != NULL);
  BlockHeader* bh = BlockHeader::GetFromObject(mem);
  EXPECT_EQ(bh->type, kLargeObject);
  scalloc::LargeObjectAllocator::Free(reinterpret_cast<LargeObjectHeader*>(bh)); 
}

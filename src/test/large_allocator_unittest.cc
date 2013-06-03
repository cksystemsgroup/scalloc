// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "allocators/large_allocator.h"
#include "runtime_vars.h"

TEST(LargeAllocator, Alloc) {
  RuntimeVars::InitModule();
  void* mem = scalloc::LargeAllocator::Alloc(512);
  EXPECT_TRUE(mem != NULL);
}

TEST(LargeAllocator, AllocFree) {
  RuntimeVars::InitModule();
  void* mem = scalloc::LargeAllocator::Alloc(512);
  EXPECT_TRUE(mem != NULL);
  BlockHeader* bh = BlockHeader::GetFromObject(mem);
  EXPECT_EQ(bh->type, kLargeObject);
  scalloc::LargeAllocator::Free(reinterpret_cast<LargeObjectHeader*>(bh)); 
}

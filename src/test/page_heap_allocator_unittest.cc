// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "allocators/global_sbrk_allocator.h"
#include "page_heap_allocator.h"
#include "runtime_vars.h"

TEST(PageHeapAllocator, Init) {
  RuntimeVars::InitModule();
  GlobalSbrkAllocator::InitModule();

  scalloc::PageHeapAllocator<uint64_t> allocator;
  allocator.Init(4096);
}

TEST(PageHeapAllocator, NewDelete) {
  RuntimeVars::InitModule();
  GlobalSbrkAllocator::InitModule();

  scalloc::PageHeapAllocator<uint64_t> allocator;
  allocator.Init(4096);
  uint64_t* v1 = allocator.New();
  EXPECT_TRUE(v1 != NULL);
  allocator.Delete(v1);
  uint64_t* v2 = allocator.New();
  EXPECT_TRUE(v2 != NULL);
  EXPECT_EQ(v1, v2);
}

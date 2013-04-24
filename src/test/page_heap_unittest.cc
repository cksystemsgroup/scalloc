// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "allocators/page_heap.h"
#include "runtime_vars.h"

namespace {

void InitHeap() {
  RuntimeVars::InitModule();
  scalloc::PageHeap::InitModule();
}

}  // namespace

TEST(PageHeap, Init) {
  InitHeap();
  scalloc::PageHeap* p = scalloc::PageHeap::GetHeap();
  EXPECT_TRUE(p != NULL);
}

TEST(PageHeap, Get) {
  InitHeap();
  scalloc::PageHeap* p = scalloc::PageHeap::GetHeap();
  void* mem = p->Get();
  EXPECT_TRUE(mem != NULL);
}

TEST(PageHeap, PutNoSegfault) {
  InitHeap();
  scalloc::PageHeap* p = scalloc::PageHeap::GetHeap();
  void* mem = p->Get();
  p->Put(mem);
}

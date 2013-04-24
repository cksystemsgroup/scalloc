// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include "runtime_vars.h"
#include "system-alloc.h"

TEST(SystemAlloc, Alloc) {
  RuntimeVars::InitModule();
  const size_t size = 16;
  size_t actual_size;
  void* mem = scalloc::SystemAlloc_Mmap(size, &actual_size);
  EXPECT_TRUE(actual_size >= size);
  EXPECT_TRUE(reinterpret_cast<uintptr_t>(mem) % RuntimeVars::SystemPageSize() == 0);
  // we could even receive NULL here
  if (mem) {}
}

TEST(SystemAlloc, Free) {
  RuntimeVars::InitModule();
  const size_t size = 16;
  size_t actual_size;
  void* mem = scalloc::SystemAlloc_Mmap(size, &actual_size);
  EXPECT_TRUE(actual_size >= size);
  scalloc::SystemFree_Mmap(mem, actual_size);
}

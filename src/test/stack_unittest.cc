// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "stack-inl.h"

TEST(Stack, EmptyInit) {
  Stack s;
  EXPECT_TRUE(s.Pop() == NULL);
}

TEST(Stack, SinglePushPop) {
  Stack s;
  void* mem;
  posix_memalign(&mem, 16, 16);
  s.Push(mem);
  EXPECT_EQ(s.Pop(), mem);
  EXPECT_TRUE(s.Pop() == NULL);
}

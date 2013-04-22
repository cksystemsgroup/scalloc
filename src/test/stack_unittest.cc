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

TEST(Stack, State) {
  Stack s;
  void* mem;
  posix_memalign(&mem, 16, 16);
  uint64_t state1 = s.GetState();
  s.Push(mem);
  uint64_t state2 = s.GetState();
  EXPECT_EQ(s.Pop(), mem);
  uint64_t state3 = s.GetState();
  EXPECT_TRUE(s.Pop() == NULL);
  uint64_t state4 = s.GetState();
  EXPECT_TRUE(state1 != state2);
  EXPECT_TRUE(state2 != state3);
  // Observing empty is non interfering.
  EXPECT_TRUE(state3 == state4);
}

TEST(Stack, PopRecordState) {
  Stack s;
  void* mem;
  posix_memalign(&mem, 16, 16);
  s.Push(mem);
  EXPECT_EQ(s.Pop(), mem);
  uint64_t state1;
  EXPECT_TRUE(s.PopRecordState(&state1) == NULL);
  s.Push(mem);
  uint64_t state2 = s.GetState();
  EXPECT_TRUE(state1 != state2);
}

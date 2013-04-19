// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "atomic.h"

TEST(TaggedAtomic, Constructors) {
  TaggedAtomic<uint64_t, uint64_t> a;
  const int alignment = a.GetAtomicAlign();
  TaggedAtomic<uint64_t, uint64_t> b(42 << alignment);
  TaggedAtomic<uint64_t, uint64_t> c(13 << alignment, 7);

  EXPECT_EQ(a.Atomic(), 0);
  EXPECT_EQ(a.Tag(), 0);
  EXPECT_EQ(b.Atomic(), 42 << alignment);
  EXPECT_EQ(b.Tag(), 0);
  EXPECT_EQ(c.Atomic(), 13 << alignment);
  EXPECT_EQ(c.Tag(), 7);
}

TEST(TaggedAtomic, CopyCtor) {
  TaggedAtomic<uint64_t, uint64_t> a;
  const int alignment = a.GetAtomicAlign();
  TaggedAtomic<uint64_t, uint64_t> b(1234 << alignment, 7);
  TaggedAtomic<uint64_t, uint64_t> c = b;
  EXPECT_EQ(c.Atomic(), 1234 << alignment);
  EXPECT_EQ(c.Tag(), 7);
}

TEST(TaggedAtomic, AssignmentOperator) {
  TaggedAtomic<uint64_t, uint64_t> a;
  const int alignment = a.GetAtomicAlign();
  TaggedAtomic<uint64_t, uint64_t> b(1234 << alignment, 7);
  TaggedAtomic<uint64_t, uint64_t> c;
  c.CopyFrom(b);
  EXPECT_EQ(c.Atomic(), 1234 << alignment);
  EXPECT_EQ(c.Tag(), 7);
}

TEST(TaggedAtomic, AtomicExchange) {
  TaggedAtomic<uint64_t, uint64_t> a;
  const int alignment = a.GetAtomicAlign();
  a.Pack(42 << alignment, 13);
  TaggedAtomic<uint64_t, uint64_t> b; // (0,0)
  TaggedAtomic<uint64_t, uint64_t> c(0, 0);
  EXPECT_TRUE(b.AtomicExchange(c, a));
  EXPECT_EQ(b.Atomic(), 42 << alignment);
  EXPECT_EQ(b.Tag(), 13);
}

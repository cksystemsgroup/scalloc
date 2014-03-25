// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "atomic.h"

TEST(TaggedValue128, Constructors) {
  TaggedValue128<int> a;
  TaggedValue128<int> b(42, 13);

  EXPECT_EQ(a.Value(), 0);
  EXPECT_EQ(a.Tag(), 0);
  EXPECT_EQ(b.Value(), 42);
  EXPECT_EQ(b.Tag(), 13);
}


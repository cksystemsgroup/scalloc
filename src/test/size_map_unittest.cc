// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include "common.h"
#include "size_map.h"

TEST(SizeMap, SizeToBlockSize) {
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(0), 0);
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(1), 16);
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(16), 16);
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(17), 32);
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(511), 512);
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(513), 1024);
  ASSERT_EQ(scalloc::SizeMap::SizeToBlockSize(1111), 1UL<<11);
}

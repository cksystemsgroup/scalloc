// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include "common.h"

TEST(Log2, values) {
  ASSERT_EQ(Log2(0), -1);
  for (unsigned i = 0; i < 32; ++i) {
  
    unsigned val = 1 << i;
    unsigned next_val = 1 << (i + 1);
    unsigned increment = (val > 64) ? (val / 16) : 1;
    
    ASSERT_EQ(Log2(val), i);
    for (unsigned j = val; j < next_val; j = j + increment) {
      ASSERT_EQ(Log2(j), i);
    }
  
  }
}

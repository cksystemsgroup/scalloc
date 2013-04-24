// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include "runtime_vars.h"

TEST(RuntimeVars, Init) {
  RuntimeVars::InitModule();
  EXPECT_TRUE(RuntimeVars::Cpus() > 0);
  EXPECT_TRUE(RuntimeVars::SystemPageSize() > 0);
}

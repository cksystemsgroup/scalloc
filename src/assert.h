// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ASSERT_H_
#define SCALLOC_ASSERT_H_

#include "log.h"

#define QUOTEME(x) #x

#ifdef DEBUG

#define ScallocAssert(c)                                                       \
  if (!(c)) {                                                                  \
    ErrorOut("assertion failed: " QUOTEME(c));                                 \
  }

#else  // !DEBUG

#define ScallocAssert(c) {}

#endif  // DEBUG

#define UNREACHABLE()                                                          \
  ErrorOut("unreachable code segment");

#endif  // SCALLOC_ASSERT_H_

// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ASSERT_H_
#define SCALLOC_ASSERT_H_

#include "log.h"

#define Fatal(format, ...) do {                                                \
  LogPrintf(kFatal, format, ##__VA_ARGS__);                                    \
  abort();                                                                     \
} while (0)

#define QUOTEME(x) #x

#ifdef DEBUG

#define ScallocAssert(c) do {                                                  \
  if (!(c)) {                                                                  \
    Fatal("assertion failed: " QUOTEME(c));                                    \
  }                                                                            \
} while (0)

#else  // !DEBUG

#define ScallocAssert(c) do {                                                  \
} while (0)

#endif  // DEBUG

#define UNREACHABLE()                                                          \
  ErrorOut("unreachable code segment");

#endif  // SCALLOC_ASSERT_H_

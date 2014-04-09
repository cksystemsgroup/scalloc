// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ASSERT_H_
#define SCALLOC_ASSERT_H_

#include "log.h"

#define Fatal(format, ...) do {                                                \
  LogPrintf("" , kFatal, format, ##__VA_ARGS__);                               \
  abort();                                                                     \
} while (0)


#define QUOTEME(x) #x


#ifdef DEBUG

#define ScallocAssert(c) do {                                                  \
  if (!(c)) {                                                                  \
    Fatal("assertion failed: " QUOTEME((c)));                                  \
  }                                                                            \
} while (0)

#else  // !DEBUG

#define ScallocAssert(c) do {                                                  \
} while (0)

#endif  // DEBUG


#define UNREACHABLE()                                                          \
  Fatal("unreachable code segment");


#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                     \
private:                                                                       \
  TypeName(const TypeName&);                                                   \
  void operator=(const TypeName&)


#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName)                               \
private:                                                                       \
  TypeName();                                                                  \
  DISALLOW_COPY_AND_ASSIGN(TypeName)


#define DISALLOW_ALLOCATION()                                                  \
public:                                                                        \
  void operator delete(void* pointer) {                                        \
    UNREACHABLE();                                                             \
  }                                                                            \
private:                                                                       \
  void* operator new(size_t size);


#endif  // SCALLOC_ASSERT_H_

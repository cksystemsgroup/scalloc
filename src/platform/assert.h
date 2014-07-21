// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "log.h"

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

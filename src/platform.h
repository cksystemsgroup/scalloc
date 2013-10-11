// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PLATFORM_H_
#define SCALLOC_PLATFORM_H_

#include <stddef.h>  // size_t

const size_t kPageShift = 12;
const size_t kPageSize = 1UL << kPageShift;

#endif  // SCALLOC_PLATFORM_H_
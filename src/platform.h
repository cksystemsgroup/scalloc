// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PLATFORM_H_
#define SCALLOC_PLATFORM_H_

#include <stddef.h>  // size_t

const size_t kPageShift = 12;
const size_t kPageSize = 1UL << kPageShift;

#define cache_aligned __attribute__((aligned(64)))

#if defined(__APPLE__)

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif  // MAP_ANONYMOUS

#endif  // __APPLE__

#endif  // SCALLOC_PLATFORM_H_

// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PLATFORM_GLOBALS_H_
#define SCALLOC_PLATFORM_GLOBALS_H_

#define UNLIKELY(x)   __builtin_expect((x), 0)
#define LIKELY(x)     __builtin_expect((x), 1)

#ifdef DEBUG
#define always_inline inline
#else
#define always_inline inline __attribute__((always_inline))
#endif  // DEBUG

#define CACHELINE_SIZE 64
#define cache_aligned __attribute__((aligned(CACHELINE_SIZE)))

const int32_t kCacheLineSize = CACHELINE_SIZE;

#ifdef __clang__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif  // __clang__

// Generally use TLS.
#define HAVE_TLS 1

#define TLS_ATTRIBUTE __thread


//
// OSX
//
#if defined(__APPLE__)

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif  // MAP_ANONYMOUS

// TLS uses malloc() on OSX, so we have to use TSD.
#undef HAVE_TLS

#endif  // __APPLE__


#endif  // SCALLOC_PLATFORM_GLOBALS_H_

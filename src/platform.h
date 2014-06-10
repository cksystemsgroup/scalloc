// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PLATFORM_H_
#define SCALLOC_PLATFORM_H_

#include <stddef.h>  // size_t

#define UNLIKELY(x)   __builtin_expect((x), 0)
#define LIKELY(x)     __builtin_expect((x), 1)

#define always_inline inline __attribute__((always_inline))

#define CACHELINE_SIZE 64
#define cache_aligned __attribute__((aligned(64)))

#define HAVE_TLS 1
#define TLS_MODE __attribute__((tls_model ("initial-exec")))

#if defined(__APPLE__)

// TLS needs malloc, so don't use it.
#undef HAVE_TLS

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif  // MAP_ANONYMOUS

#endif  // __APPLE__

const size_t kPageShift = 12;
const size_t kPageSize = 1UL << kPageShift;

// Prohibit reordering of instructions by the compiler.
always_inline void CompilerBarrier() {
  __asm__ __volatile__("" : : : "memory");
}

// Full memory fence on x86-64
always_inline void MemoryBarrier() {
  __asm__ __volatile__("mfence" : : : "memory");
}

#define PTR(p) reinterpret_cast<void*>((p))

#endif  // SCALLOC_PLATFORM_H_

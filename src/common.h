// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_COMMON_H_
#define SCALLOC_COMMON_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // size_t

#include "assert.h"
#include "platform.h"

#define UNLIKELY(x)   __builtin_expect((x), 0)
#define LIKELY(x)     __builtin_expect((x), 1)

#define always_inline inline __attribute__((always_inline))
#define no_inline __attribute__((noinline))

#define TLS_MODE __attribute__((tls_model ("initial-exec")))

#define HAVE_TLS 1
#if defined(__APPLE__)
#undef HAVE_TLS
#endif  // __APPLE__

const size_t kMinAlignment = 16;
const size_t kMaxSmallShift = 8;  // up to 2MiB
const size_t kMaxSmallSize = 1UL << kMaxSmallShift;
const size_t kMaxMediumShift = 21;  // up to 2MiB
const size_t kMaxMediumSize = 1UL << kMaxMediumShift;

const size_t kVirtualSpanShift = 22;  // 4MiB
const size_t kVirtualSpanSize = 1UL << kVirtualSpanShift;
const uintptr_t kVirtualSpanMask = ~(kVirtualSpanSize - 1);

const size_t kFineClasses = kMaxSmallSize / kMinAlignment + 1;
const size_t kCoarseClasses = kMaxMediumShift - kMaxSmallShift;
const size_t kNumClasses = kFineClasses + kCoarseClasses;

#ifdef SMALL_SPACE_SIZE
const size_t kSmallSpace = SMALL_SPACE_SIZE;
const size_t kInternalSpace = 1UL << 31;  // 2GiB
#elif defined  __x86_64__
const size_t kSmallSpace = 1UL << 45;  // 32TiB
const size_t kInternalSpace = 1UL << 31;  // 2GiB
#elif defined __i386__
const size_t kSmallSpace = 1UL << 31;  // 2GiB
const size_t kInternalSpace = 1UL << 31;  // 2GiB
#else
#error "platform not supported"
#endif

const size_t kSpanReuseThreshold = SPAN_REUSE_THRESHOLD;
const size_t kLocalReuseThreshold = LOCAL_REUSE_THRESHOLD;

// Prohibit reordering of instructions by the compiler.
inline void CompilerBarrier() {
  __asm__ __volatile__("" : : : "memory");
}

// Full memory fence on x86-64
inline void MemoryBarrier() {
  __asm__ __volatile__("mfence" : : : "memory");
}

#endif  // SCALLOC_COMMON_H_

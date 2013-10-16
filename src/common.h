// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_COMMON_H_
#define SCALLOC_COMMON_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // size_t

#include "log.h"
#include "platform.h"

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

#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
static const char log_table[256] = {
  -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
  LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

// base-2 logarithm of 32-bit integers
inline int Log2(size_t v) {
  unsigned int t, tt, r;  // temp vars
  if ((tt = (v >> 16))) {
    r =  (t = (tt >> 8)) ? 24 + log_table[t] : 16 + log_table[tt];
  } else {
    r =  (t = (v >> 8)) ? 8 + log_table[t] : log_table[v];
  }
  return r;
}

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

inline size_t PadSize(size_t size, size_t multiple) {
  return (size + multiple - 1) / multiple * multiple;
}

#endif  // SCALLOC_COMMON_H_

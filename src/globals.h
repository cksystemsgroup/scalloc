// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_GLOBALS_H_
#define SCALLOC_GLOBALS_H_

#include "platform/globals.h"

const size_t kPageSize = 4096;
const uint64_t kPageNrMask = ~(static_cast<uint64_t>(kPageSize) - 1);
const uint64_t kPageOffsetMask = static_cast<uint64_t>(kPageSize) - 1;

const size_t kMaxThreads = 128;

const uint64_t kKilo = 1UL << 10;
const uint64_t kMega = kKilo * kKilo;
const uint64_t kGiga = kMega * kKilo;
const uint64_t kTera = kGiga * kKilo;

const uint64_t kLABSpaceSize = 100 * kPageSize;
const uint64_t kObjectSpaceSize = 35 * kTera;

// TODO: Cleanup.
const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 20;  // 1MiB
const size_t kVirtualSpanShift = 21;  // 2MiB

const size_t kMinAlignment = 16;
const size_t kMaxSmallSize = 1UL << kMaxSmallShift;
const size_t kMaxMediumSize = 1UL << kMaxMediumShift;
const size_t kVirtualSpanSize = 1UL << kVirtualSpanShift;
const uintptr_t kVirtualSpanMask = ~(kVirtualSpanSize - 1);
const size_t kFineClasses = kMaxSmallSize / kMinAlignment + 1;
const size_t kCoarseClasses = kMaxMediumShift - kMaxSmallShift;
const int32_t kNumClasses = kFineClasses + kCoarseClasses;

namespace scalloc {

// Some defaults.

#ifndef SCALLOC_REUSE_THRESHOLD
#define SCALLOC_REUSE_THRESHOLD (80)
#endif  // SCALLOC_REUSE_THRESHOLD

#define SCALLOC_LAB_MODEL_TLAB  0
#define SCALLOC_LAB_MODEL_RR    1
#ifndef SCALLOC_LAB_MODEL
#define SCALLOC_LAB_MODEL SCALLOC_LAB_MODEL_TLAB
#endif  // SCALLOC_LAB_MODEL

#ifndef SCALLOC_NO_MADVISE
#define SCALLOC_MADVISE 1
#endif  // !SCALLOC_NO_MADVISE

#ifndef SCALLOC_NO_MADVISE_EAGER
#define SCALLOC_MADVISE_EAGER 1
#endif  // !SCALLOC_NO_MADVISE_EAGER

const int32_t kReuseThreshold = SCALLOC_REUSE_THRESHOLD;

#if SCALLOC_LAB_MODEL == SCALLOC_LAB_MODEL_TLAB
class ThreadLocalAllocationBuffer;
typedef ThreadLocalAllocationBuffer ABProvider;
#elif SCALLOC_LAB_MODEL == SCALLOC_LAB_MODEL_RR
class RoundRobinAllocationBuffer;
typedef RoundRobinAllocationBuffer ABProvider;
#else
#error "unknown LAB model"
#endif  // SCALLOC_LAB_MODEL

class Arena;
class SpanPool;

extern Arena object_space;
extern Arena core_space;
extern SpanPool span_pool;
extern ABProvider ab_scheduler;

}  // namespace scalloc

#endif  // SCALLOC_GLOBALS_H_

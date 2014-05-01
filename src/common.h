// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_COMMON_H_
#define SCALLOC_COMMON_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // size_t

#include "platform.h"

#ifdef HUGE_PAGE

#define HUGEPAGE_SIZE (1UL << 21)
#define VSPAN_SIZE 131072

const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 16;  // 64k
const size_t kVirtualSpanShift = 17;  // 128k
#else  // no huge pages
#ifdef SZ_1MB
const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 19;  // 1MiB
const size_t kVirtualSpanShift = 20;  // 2MiB
#else
const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 20;  // 1MiB
const size_t kVirtualSpanShift = 21;  // 2MiB
#endif  // SZ_1MB
#endif  // HUGE_PAGE

const size_t kMinAlignment = 16;
const size_t kMaxSmallSize = 1UL << kMaxSmallShift;
const size_t kMaxMediumSize = 1UL << kMaxMediumShift;
const size_t kVirtualSpanSize = 1UL << kVirtualSpanShift;
const uintptr_t kVirtualSpanMask = ~(kVirtualSpanSize - 1);
const size_t kFineClasses = kMaxSmallSize / kMinAlignment + 1;
const size_t kCoarseClasses = kMaxMediumShift - kMaxSmallShift;
const size_t kNumClasses = kFineClasses + kCoarseClasses;

#if defined(__x86_64__)

#ifdef SMALL_SPACE
const size_t kSmallSpace = SMALL_SPACE;
#else
#ifdef HUGE_PAGE
#ifdef HUGE_PAGE_SPACE
const size_t kSmallSpace = HUGE_PAGE_SPACE;
#else
const size_t kSmallSpace = (1UL << 36) + (1UL << 35);  // 96GiB
#endif  // HUGE_PAGE_SPACE
#else
const size_t kSmallSpace = 1UL << 44;  // 16TiB
#endif  // HUGE_PAGE
#endif
const size_t kInternalSpace = 1UL << 31;  // 2GiB

#else
#error "platform not supported"
#endif

#ifndef SPAN_REUSE_THRESHOLD
#define SPAN_REUSE_THRESHOLD 80
#endif  // SPAN_REUSE_THRESHOLD

#ifndef LOCAL_REUSE_THRESHOLD
#define LOCAL_REUSE_THRESHOLD 80
#endif  // LOCAL_REUSE_THRESHOLD

const size_t kSpanReuseThreshold = SPAN_REUSE_THRESHOLD;
const size_t kLocalReuseThreshold = LOCAL_REUSE_THRESHOLD;
#ifdef EAGER_MADVISE_THRESHOLD
#define EAGER_MADVISE 1
const size_t kEagerMadviseThreshold = EAGER_MADVISE_THRESHOLD;
#ifdef MADVISE_SEPARATE_THREAD
#define COLLECTOR 1
#endif  // MADVISE_SEPARATE_THREAD
#endif  // EAGER_MADVISE_THRESHOLD

#ifndef POLICY_CORE_LOCAL
#define POLICY_THREAD_LOCAL
#endif  // POLICY_CORE_LOCAL

#ifndef MAX_PARALLELISM
#define MAX_PARALLELISM 80
#endif  // MAX_PARALLELISM

#if !defined(CLAB_UTILIZATION) && !defined(CLAB_THREADS) && !defined(CLAB_RR)
#define CLAB_UTILIZATION 1
#endif  // !CLAB_UTILIZATION && !CLAB_THREADS && !CLAB_RR

namespace scalloc {

enum LockMode {
  kLocal = 0,
  kSizeClassLocked
};

}  // namespace scalloc



#endif  // SCALLOC_COMMON_H_

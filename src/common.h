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

const size_t kMaxSmallShift = 8;  // up to 256B
const size_t kMaxMediumShift = 10;  // up to 4k
const size_t kVirtualSpanShift = 21;  // 2MiB
#else  // no huge pages
const size_t kMaxSmallShift = 8;  // up to 256B
const size_t kMaxMediumShift = 21;  // up to 2MiB
const size_t kVirtualSpanShift = 22;  // 4MiB
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


namespace scalloc {

enum LockMode {
  kLocal = 0,
  kSizeClassLocked
};

}  // namespace scalloc



#endif  // SCALLOC_COMMON_H_

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_COMMON_H_
#define SCALLOC_COMMON_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // size_t

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

#if defined(__x86_64__)

#ifdef SMALL_SPACE
const size_t kSmallSpace = SMALL_SPACE;
#else
const size_t kSmallSpace = 1UL << 45;  // 32TiB
#endif
const size_t kInternalSpace = 1UL << 31;  // 2GiB

#else
#error "platform not supported"
#endif

const size_t kSpanReuseThreshold = SPAN_REUSE_THRESHOLD;
const size_t kLocalReuseThreshold = LOCAL_REUSE_THRESHOLD;
#ifdef EAGER_MADVISE_THRESHOLD
#define EAGER_MADVISE 1
const size_t kEagerMadviseThreshold = EAGER_MADVISE_THRESHOLD;
#ifdef MADVISE_SEPARATE_THREAD
#define COLLECTOR 1
#endif  // MADVISE_SEPARATE_THREAD
#endif  // EAGER_MADVISE_THRESHOLD

#endif  // SCALLOC_COMMON_H_

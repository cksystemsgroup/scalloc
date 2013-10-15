// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_COMMON_H_
#define SCALLOC_COMMON_H_

#include <cstddef>  // size_t
#include <cstdint>  // uintptr_t

#include "platform.h"

#if defined(__x86_64__)
const size_t kSmallSpace = 1UL << 45;  // 32TiB
const size_t kInternalSpace = 1UL << 31;  // 2GiB
#else
#error "platform not supported"
#endif

const size_t kMinAlignment = 16;

const size_t kMaxFineShift = 8;  // 256B
const size_t kMaxFineSize = 1UL << kMaxFineShift;
const size_t kFineClasses = kMaxFineSize / kMinAlignment + 1;
const size_t kMaxSmallShift = 21;  // 2MB
const size_t kMaxSmallSize = 1UL << kMaxSmallShift;
const size_t kCoarseClasses = kMaxSmallShift - kMaxFineShift;
const size_t kNumClasses = kFineClasses + kCoarseClasses;

const size_t kVirtualSpanShift = 22;  // 4MiB
const size_t kVirtualSpanSize = 1UL << kVirtualSpanShift;
const uintptr_t kVirtualSpanMask = ~(kVirtualSpanSize - 1);

const size_t kSpanReuseThreshold = SPAN_REUSE_THRESHOLD;
const size_t kLocalReuseThreshold = LOCAL_REUSE_THRESHOLD;

#endif  // SCALLOC_COMMON_H_

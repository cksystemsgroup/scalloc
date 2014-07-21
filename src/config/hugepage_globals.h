// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_CONFIG_HUGEPAGE_GLOBALS_H_
#define SCALLOC_CONFIG_HUGEPAGE_GLOBALS_H_

#define HUGEPAGE_SIZE (1UL << 21)
#define VSPAN_SIZE 131072

const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 16;  // 64k
const size_t kVirtualSpanShift = 17;  // 128k

#endif  // SCALLOC_CONFIG_HUGEPAGE_GLOBALS_H_

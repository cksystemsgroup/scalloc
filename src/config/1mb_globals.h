// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_CONFIG_1MB_GLOBALS_H_
#define SCALLOC_CONFIG_1MB_GLOBALS_H_

const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 19;  // 500k
const size_t kVirtualSpanShift = 20;  // 1MiB

#endif  // SCALLOC_CONFIG_1MB_GLOBALS_H_

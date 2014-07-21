// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_CONFIG_DEFAULT_GLOBALS_H_
#define SCALLOC_CONFIG_DEFAULT_GLOBALS_H_

const size_t kMaxSmallShift = 8;  // 256B
const size_t kMaxMediumShift = 20;  // 1MiB
const size_t kVirtualSpanShift = 21;  // 2MiB

#endif  // SCALLOC_CONFIG_DEFAULT_GLOBALS_H_
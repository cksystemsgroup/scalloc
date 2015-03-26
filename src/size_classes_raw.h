// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

//
//                          +----------------+
//                          |  DO NOT EDIT!  |
//                          +----------------+
//
// This file is auto-generated using ``tools/gen_size_classes.py huge''

#ifndef SCALLOC_SIZE_CLASSES_RAW_H_
#define SCALLOC_SIZE_CLASSES_RAW_H_

const int32_t kSpanHeaderSize = 128;

#define FOR_ALL_SIZE_CLASSES(V) \
  V(0, 0, 0, 0) /* NOLINT */ \
  V(1, 16, 32768, (32768 - kSpanHeaderSize)/16) /* NOLINT */ \
  V(2, 32, 32768, (32768 - kSpanHeaderSize)/32) /* NOLINT */ \
  V(3, 48, 32768, (32768 - kSpanHeaderSize)/48) /* NOLINT */ \
  V(4, 64, 32768, (32768 - kSpanHeaderSize)/64) /* NOLINT */ \
  V(5, 80, 32768, (32768 - kSpanHeaderSize)/80) /* NOLINT */ \
  V(6, 96, 32768, (32768 - kSpanHeaderSize)/96) /* NOLINT */ \
  V(7, 112, 32768, (32768 - kSpanHeaderSize)/112) /* NOLINT */ \
  V(8, 128, 32768, (32768 - kSpanHeaderSize)/128) /* NOLINT */ \
  V(9, 144, 32768, (32768 - kSpanHeaderSize)/144) /* NOLINT */ \
  V(10, 160, 32768, (32768 - kSpanHeaderSize)/160) /* NOLINT */ \
  V(11, 176, 32768, (32768 - kSpanHeaderSize)/176) /* NOLINT */ \
  V(12, 192, 32768, (32768 - kSpanHeaderSize)/192) /* NOLINT */ \
  V(13, 208, 32768, (32768 - kSpanHeaderSize)/208) /* NOLINT */ \
  V(14, 224, 32768, (32768 - kSpanHeaderSize)/224) /* NOLINT */ \
  V(15, 240, 32768, (32768 - kSpanHeaderSize)/240) /* NOLINT */ \
  V(16, 256, 32768, (32768 - kSpanHeaderSize)/256) /* NOLINT */ \
  V(17, 512, ((64 * 512 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 64) /* NOLINT */ \
  V(18, 1024, ((64 * 1024 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 64) /* NOLINT */ \
  V(19, 2048, ((64 * 2048 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 64) /* NOLINT */ \
  V(20, 4096, ((32 * 4096 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 32) /* NOLINT */ \
  V(21, 8192, ((32 * 8192 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 32) /* NOLINT */ \
  V(22, 16384, ((16 * 16384 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 16) /* NOLINT */ \
  V(23, 32768, ((16 * 32768 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 16) /* NOLINT */ \
  V(24, 65536, ((16 * 65536 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 16) /* NOLINT */ \
  V(25, 131072, ((8 * 131072 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 8) /* NOLINT */ \
  V(26, 262144, ((4 * 262144 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  V(27, 524288, ((2 * 524288 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 2) /* NOLINT */ \
  V(28, 1048576, ((1 * 1048576 + kSpanHeaderSize)/kPageSize + 1) * kPageSize, 1) /* NOLINT */

#endif  // SCALLOC_SIZE_CLASSES_RAW_H_

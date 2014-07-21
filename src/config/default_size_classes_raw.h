// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
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

#define SIZE_CLASSES \
  SIZE_CLASS(0, 0, 0, 0) /* NOLINT */ \
  SIZE_CLASS(1, 16, 16384, (16384 - sizeof(SpanHeader))/16) /* NOLINT */ \
  SIZE_CLASS(2, 32, 16384, (16384 - sizeof(SpanHeader))/32) /* NOLINT */ \
  SIZE_CLASS(3, 48, 16384, (16384 - sizeof(SpanHeader))/48) /* NOLINT */ \
  SIZE_CLASS(4, 64, 16384, (16384 - sizeof(SpanHeader))/64) /* NOLINT */ \
  SIZE_CLASS(5, 80, 16384, (16384 - sizeof(SpanHeader))/80) /* NOLINT */ \
  SIZE_CLASS(6, 96, 16384, (16384 - sizeof(SpanHeader))/96) /* NOLINT */ \
  SIZE_CLASS(7, 112, 16384, (16384 - sizeof(SpanHeader))/112) /* NOLINT */ \
  SIZE_CLASS(8, 128, 32768, (32768 - sizeof(SpanHeader))/128) /* NOLINT */ \
  SIZE_CLASS(9, 144, 32768, (32768 - sizeof(SpanHeader))/144) /* NOLINT */ \
  SIZE_CLASS(10, 160, 32768, (32768 - sizeof(SpanHeader))/160) /* NOLINT */ \
  SIZE_CLASS(11, 176, 32768, (32768 - sizeof(SpanHeader))/176) /* NOLINT */ \
  SIZE_CLASS(12, 192, 32768, (32768 - sizeof(SpanHeader))/192) /* NOLINT */ \
  SIZE_CLASS(13, 208, 32768, (32768 - sizeof(SpanHeader))/208) /* NOLINT */ \
  SIZE_CLASS(14, 224, 32768, (32768 - sizeof(SpanHeader))/224) /* NOLINT */ \
  SIZE_CLASS(15, 240, 32768, (32768 - sizeof(SpanHeader))/240) /* NOLINT */ \
  SIZE_CLASS(16, 256, 32768, (32768 - sizeof(SpanHeader))/256) /* NOLINT */ \
  SIZE_CLASS(17, 512, ((32 * 512 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 32) /* NOLINT */ \
  SIZE_CLASS(18, 1024, ((32 * 1024 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 32) /* NOLINT */ \
  SIZE_CLASS(19, 2048, ((16 * 2048 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 16) /* NOLINT */ \
  SIZE_CLASS(20, 4096, ((4 * 4096 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(21, 8192, ((4 * 8192 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(22, 16384, ((4 * 16384 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(23, 32768, ((4 * 32768 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(24, 65536, ((2 * 65536 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 2) /* NOLINT */ \
  SIZE_CLASS(25, 131072, ((2 * 131072 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 2) /* NOLINT */ \
  SIZE_CLASS(26, 262144, ((2 * 262144 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 2) /* NOLINT */ \
  SIZE_CLASS(27, 524288, ((2 * 524288 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 2) /* NOLINT */ \
  SIZE_CLASS(28, 1048576, ((1 * 1048576 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 1) /* NOLINT */

#endif  // SCALLOC_SIZE_CLASSES_RAW_H_

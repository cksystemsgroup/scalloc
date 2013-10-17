// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

// This file is auto-generated using ``python tools/gen_size_classes.py''

#ifndef SCALLOC_SIZE_CLASSES_RAW_H_
#define SCALLOC_SIZE_CLASSES_RAW_H_

#define SIZE_CLASSES \
  SIZE_CLASS(0, 0, 0, 0) \ /* NOLINT */ \
  SIZE_CLASS(1, 16, 8192, (8192 - sizeof(SpanHeader))/16) /* NOLINT */ \
  SIZE_CLASS(2, 32, 8192, (8192 - sizeof(SpanHeader))/32) /* NOLINT */ \
  SIZE_CLASS(3, 48, 8192, (8192 - sizeof(SpanHeader))/48) /* NOLINT */ \
  SIZE_CLASS(4, 64, 8192, (8192 - sizeof(SpanHeader))/64) /* NOLINT */ \
  SIZE_CLASS(5, 80, 8192, (8192 - sizeof(SpanHeader))/80) /* NOLINT */ \
  SIZE_CLASS(6, 96, 8192, (8192 - sizeof(SpanHeader))/96) /* NOLINT */ \
  SIZE_CLASS(7, 112, 8192, (8192 - sizeof(SpanHeader))/112) /* NOLINT */ \
  SIZE_CLASS(8, 128, 16384, (16384 - sizeof(SpanHeader))/128) /* NOLINT */ \
  SIZE_CLASS(9, 144, 16384, (16384 - sizeof(SpanHeader))/144) /* NOLINT */ \
  SIZE_CLASS(10, 160, 16384, (16384 - sizeof(SpanHeader))/160) /* NOLINT */ \
  SIZE_CLASS(11, 176, 16384, (16384 - sizeof(SpanHeader))/176) /* NOLINT */ \
  SIZE_CLASS(12, 192, 16384, (16384 - sizeof(SpanHeader))/192) /* NOLINT */ \
  SIZE_CLASS(13, 208, 16384, (16384 - sizeof(SpanHeader))/208) /* NOLINT */ \
  SIZE_CLASS(14, 224, 16384, (16384 - sizeof(SpanHeader))/224) /* NOLINT */ \
  SIZE_CLASS(15, 240, 16384, (16384 - sizeof(SpanHeader))/240) /* NOLINT */ \
  SIZE_CLASS(16, 256, 16384, (16384 - sizeof(SpanHeader))/256) /* NOLINT */ \
  SIZE_CLASS(17, 512, ((64 * 512 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 64) /* NOLINT */ \
  SIZE_CLASS(18, 1024, ((32 * 1024 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 32) /* NOLINT */ \
  SIZE_CLASS(19, 2048, ((16 * 2048 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 16) /* NOLINT */ \
  SIZE_CLASS(20, 4096, ((8 * 4096 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 8) /* NOLINT */ \
  SIZE_CLASS(21, 8192, ((8 * 8192 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 8) /* NOLINT */ \
  SIZE_CLASS(22, 16384, ((8 * 16384 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 8) /* NOLINT */ \
  SIZE_CLASS(23, 32768, ((8 * 32768 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 8) /* NOLINT */ \
  SIZE_CLASS(24, 65536, ((4 * 65536 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(25, 131072, ((4 * 131072 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(26, 262144, ((4 * 262144 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(27, 524288, ((4 * 524288 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 4) /* NOLINT */ \
  SIZE_CLASS(28, 1048576, ((2 * 1048576 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 2) /* NOLINT */ \
  SIZE_CLASS(29, 2097152, ((1 * 2097152 + sizeof(SpanHeader))/kPageSize + 1) * kPageSize, 1) /* NOLINT */

#endif  // SCALLOC_SIZE_CLASSES_RAW_H_

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

//
//                          +----------------+
//                          |  DO NOT EDIT!  |
//                          +----------------+
//
// This file is auto-generated using ``tools/gen_size_classes.py huge''

#ifndef SCALLOC_SIZE_CLASSES_RAW_HUGEPAGE_H_
#define SCALLOC_SIZE_CLASSES_RAW_HUGEPAGE_H_

#define SIZE_CLASSES \
  SIZE_CLASS(0, 0, 0, 0) /* NOLINT */ \
  SIZE_CLASS(1, 16, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/16)) /* NOLINT */ \
  SIZE_CLASS(2, 32, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/32)) /* NOLINT */ \
  SIZE_CLASS(3, 48, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/48)) /* NOLINT */ \
  SIZE_CLASS(4, 64, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/64)) /* NOLINT */ \
  SIZE_CLASS(5, 80, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/80)) /* NOLINT */ \
  SIZE_CLASS(6, 96, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/96)) /* NOLINT */ \
  SIZE_CLASS(7, 112, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/112)) /* NOLINT */ \
  SIZE_CLASS(8, 128, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/128)) /* NOLINT */ \
  SIZE_CLASS(9, 144, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/144)) /* NOLINT */ \
  SIZE_CLASS(10, 160, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/160)) /* NOLINT */ \
  SIZE_CLASS(11, 176, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/176)) /* NOLINT */ \
  SIZE_CLASS(12, 192, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/192)) /* NOLINT */ \
  SIZE_CLASS(13, 208, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/208)) /* NOLINT */ \
  SIZE_CLASS(14, 224, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/224)) /* NOLINT */ \
  SIZE_CLASS(15, 240, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/240)) /* NOLINT */ \
  SIZE_CLASS(16, 256, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/256)) /* NOLINT */ \
  SIZE_CLASS(17, 512, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/512)) /* NOLINT */ \
  SIZE_CLASS(18, 1024, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/1024)) /* NOLINT */ \
  SIZE_CLASS(19, 2048, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/2048)) /* NOLINT */ \
  SIZE_CLASS(20, 4096, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/4096)) /* NOLINT */ \
  SIZE_CLASS(21, 8192, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/8192)) /* NOLINT */ \
  SIZE_CLASS(22, 16384, HUGEPAGE_SIZE, ((HUGEPAGE_SIZE - sizeof(SpanHeader))/16384)) /* NOLINT */

#endif  // SCALLOC_SIZE_CLASSES_RAW_HUGEPAGE_H_

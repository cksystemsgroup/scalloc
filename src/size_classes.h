// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SIZE_CLASSES_H_
#define SCALLOC_SIZE_CLASSES_H_

#include <cinttypes>
#include <cstdio>

#include "globals.h"
#include "platform/assert.h"
#include "platform/globals.h"
#include "utils.h"

namespace scalloc {

extern const int32_t ClassToObjects[];
extern const int32_t ClassToSize[];
extern const int32_t ClassToSpanSize[];
extern const int32_t ClassToReuseThreshold[];

always_inline int32_t SizeToClass(const size_t size) __attribute__((pure));
always_inline int32_t SizeToBlockSize(const size_t size) __attribute__((pure));


int32_t SizeToClass(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) / kMinAlignment;
  }
  if (size <= kMaxMediumSize) {
    return Log2(size - 1) - kMaxSmallShift + kFineClasses;
  }
  // 0 indicates size of 0 or large objects.
  return 0;
}


int32_t SizeToBlockSize(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) & ~(kMinAlignment-1);
  } else if (size <= kMaxMediumSize) {
    return 1UL << (Log2(size - 1) + 1);
  }
  UNREACHABLE();
  return 0;
}


inline void PrintSizeclasses() {
  fprintf(stderr, "Sizeclass summary\n");
  fprintf(stderr, "Reuse threshold: %d%%\n", kReuseThreshold);
  int32_t waste = 0;
  for (int32_t i = 0; i < kNumClasses; i++) {
    if (i > 0) {
      waste = ClassToSize[i] - ClassToSize[i - 1] - 1;
      waste = (waste * 100) / ClassToSize[i];
    }
    fprintf(stderr, "[9] \t[%d] "
           "size: %d, "
           "objects: %d, "
           "realspan size: %d, "
           "reuse threshold: %d, "
           "waste: %d%%\n",
           i,
           ClassToSize[i],
           ClassToObjects[i],
           ClassToSpanSize[i],
           ClassToReuseThreshold[i],
           waste);
  }
  abort();
}

}  // namespace scalloc

#endif  // SCALLOC_SIZE_CLASSES_H_


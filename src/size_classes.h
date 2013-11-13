// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SIZE_CLASSES_H_
#define SCALLOC_SIZE_CLASSES_H_

#include <cinttypes>
#include <cstdio>

#include "assert.h"
#include "common.h"
#include "utils.h"

namespace scalloc {

extern const uint64_t ClassToObjects[];
extern const uint64_t ClassToSize[];
extern const uint64_t ClassToSpanSize[];


inline size_t SizeToClass(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) / kMinAlignment;
  }
  if (size <= kMaxMediumSize) {
    return utils::Log2(size - 1) - kMaxSmallShift + kFineClasses;
  }
  UNREACHABLE();
  return 0;
}


inline size_t SizeToBlockSize(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) & ~(kMinAlignment-1);
  } else if (size <= kMaxMediumSize) {
    return 1UL << (utils::Log2(size - 1) + 1);
  }
  UNREACHABLE();
  return 0;
}

inline void PrintSizeclasses() {
  printf("Sizeclass summary\n");
  for (size_t i = 0; i < kNumClasses; i++) {
    printf("\t[%lu] "
           "size: %" PRIu64 ", "
           "objects: %" PRIu64 ", "
           "realspan size: %" PRIu64 "\n",
           i, ClassToSize[i], ClassToObjects[i], ClassToSpanSize[i]);
  }
}


}  // namespace scalloc

#endif  // SCALLOC_SIZE_CLASSES_H_

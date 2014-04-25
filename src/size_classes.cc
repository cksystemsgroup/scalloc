// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "size_classes.h"

#include "headers.h"

#ifdef HUGE_PAGE
#include "size_classes_raw_hugepage.h"
#else
#ifdef SZ_2MB
#include "size_classes_raw_2MB.h"
#else
#include "size_classes_raw.h"
#endif  // SZ_2MB
#endif  // HUGE_PAGE

namespace scalloc {

cache_aligned const uint64_t ClassToObjects[] = {
#define SIZE_CLASS(a, b, c, d) (d),
SIZE_CLASSES
#undef SIZE_CLASS
};

cache_aligned const uint64_t ClassToSize[] = {
#define SIZE_CLASS(a, b, c, d) (b),
SIZE_CLASSES
#undef SIZE_CLASS
};

cache_aligned const uint64_t ClassToSpanSize[] = {
#define SIZE_CLASS(a, b, c, d) (c),
SIZE_CLASSES
#undef SIZE_CLASS
};

void CheckSizeClasses() {
  uint64_t payload;
  for (size_t i = 1; i < kNumClasses; i++) {
    payload = ClassToSize[i] * ClassToObjects[i];
    if ((payload + sizeof(SpanHeader)) > ClassToSpanSize[i]) {
      LOG(kError, "size class: %lu, size: %lu, objects: %lu, span size: %lu",
          i, ClassToSize[i], ClassToObjects[i], ClassToSpanSize[i]);
      Fatal("inconsistent size classes");
    }
  }
}

}  // namespace scalloc


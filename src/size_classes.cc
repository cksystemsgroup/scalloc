// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "size_classes.h"

#include "headers.h"

namespace scalloc {

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


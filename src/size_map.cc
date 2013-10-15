// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "size_map.h"

#include "block_header.h"
#include "common.h"

namespace scalloc {

SizeMap SizeMap::size_map_;

void SizeMap::Init() {
  SizeMap& sm = Instance();
  
  sm.class_to_size_[0] = 0;
  sm.class_to_objs_[0] = 0;
  sm.class_to_span_size_[0] = 0;
  
  // fine classes
  size_t smallest_real_span_size = 1UL << 13;  // 8K
  for (size_t i = 1; i < (kFineClasses / 2); i++) {
    sm.class_to_size_[i] = sm.class_to_size_[i-1] + kMinAlignment;
    sm.class_to_span_size_[i] = smallest_real_span_size;
    sm.class_to_objs_[i] =
        (sm.class_to_span_size_[i] - sizeof(SpanHeader)) / sm.class_to_size_[i];
  }
  
  // fine classes, but more larger spans
  smallest_real_span_size = 1UL << 14;  // 16K
  for (size_t i = (kFineClasses / 2); i < kFineClasses; i++) {
    sm.class_to_size_[i] = sm.class_to_size_[i-1] + kMinAlignment;
    sm.class_to_span_size_[i] = smallest_real_span_size;
    sm.class_to_objs_[i] =
        (sm.class_to_span_size_[i] - sizeof(SpanHeader)) / sm.class_to_size_[i];
  }
  
  // coarser classes

  // we start with 1KB objects
  sm.class_to_objs_[kFineClasses+0] = 1UL << 6;  // 512B blocks
  sm.class_to_objs_[kFineClasses+1] = 1UL << 5;  // 1KB blocks
  sm.class_to_objs_[kFineClasses+2] = 1UL << 4;  // 2KB blocks
  sm.class_to_objs_[kFineClasses+3] = 1UL << 3;  // 4KB blocks
  sm.class_to_objs_[kFineClasses+4] = 1UL << 3;  // 8KB blocks
  sm.class_to_objs_[kFineClasses+5] = 1UL << 3;  // 16KB blocks
  sm.class_to_objs_[kFineClasses+6] = 1UL << 3;  // 32KB blocks
  sm.class_to_objs_[kFineClasses+7] = 1UL << 2;  // 64KB blocks
  sm.class_to_objs_[kFineClasses+8] = 1UL << 2;  // 128KB blocks
  sm.class_to_objs_[kFineClasses+9] = 1UL << 2;  // 256KB blocks
  sm.class_to_objs_[kFineClasses+10] = 1UL << 2;  // 512KB blocks
  sm.class_to_objs_[kFineClasses+11] = 1UL << 1;  // 1MB blocks
  sm.class_to_objs_[kFineClasses+12] = 1UL << 0;  // 2MB blocks

  // set block size for >512B size classes
  for (size_t i = kFineClasses; i < kNumClasses; i++) {
    sm.class_to_size_[i] = sm.class_to_size_[i-1] * 2;
    sm.class_to_span_size_[i] =
        ((sm.class_to_objs_[i] * sm.class_to_size_[i] + sizeof(SpanHeader))
         / kPageSize + 1) * kPageSize;
  }
}

}  // namespace scalloc

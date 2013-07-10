// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "size_map.h"

#include "block_header.h"
#include "common.h"
#include "runtime_vars.h"

namespace scalloc {

void SizeMap::InitModule() {
  SizeMap::Instance().Init();
}

void SizeMap::Init() {
  class_to_size_[0] = 0;
  class_to_objs_[0] = 0;
  class_to_span_size_[0] = 0;
  size_t smallest_real_span_size = 1UL << 15; // 32K
  for (size_t i = 1; i < kFineClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] + kMinAlignment;
    class_to_span_size_[i] = smallest_real_span_size;
    class_to_objs_[i] = (class_to_span_size_[i] - sizeof(SpanHeader)) / class_to_size_[i];
  }
  //we start with 1KB objects
  class_to_objs_[kFineClasses+0] =    1UL << 6; // 512 B blocks
  class_to_objs_[kFineClasses+1] =    1UL << 5; // 1KB blocks
  class_to_objs_[kFineClasses+2] =  1UL << 4; // 2KB blocks
  class_to_objs_[kFineClasses+3] =  1UL << 3; // 4KB blocks
  class_to_objs_[kFineClasses+4] =  1UL << 3; // 8KB blocks
  class_to_objs_[kFineClasses+5] =  1UL << 3; // 16KB blocks
  class_to_objs_[kFineClasses+6] =  1UL << 3; // 32KB blocks
  class_to_objs_[kFineClasses+7] =  1UL << 2; // 64KB blocks
  class_to_objs_[kFineClasses+8] =  1UL << 2; // 128KB blocks
  class_to_objs_[kFineClasses+9] =  1UL << 2; // 256KB blocks
  class_to_objs_[kFineClasses+10] =  1UL << 2; // 512KB blocks
  class_to_objs_[kFineClasses+11] = 1UL << 1; // 1MB blocks
  class_to_objs_[kFineClasses+12] = 1UL << 1; // 2MB blocks


  //size_t large_objects_per_sc = 1UL<<15; // start with 8k objects in sc 1KB and half them in each round
  for (size_t i = kFineClasses; i < kNumClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] * 2; 
    //class_to_objs_[i] = large_objects_per_sc;
    class_to_span_size_[i] = ((class_to_objs_[i] * class_to_size_[i] + sizeof(SpanHeader))  / RuntimeVars::SystemPageSize() + 1) * RuntimeVars::SystemPageSize();

    //if (large_objects_per_sc > (1UL<<6)) {
    //  large_objects_per_sc = large_objects_per_sc / 2;
    //}
  }
}

}  // namespace scalloc

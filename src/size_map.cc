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
  for (size_t i = 1; i < kFineClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] + kMinAlignment;
    const size_t sys_page_sz = RuntimeVars::SystemPageSize();
    // TODO: Fix class_to_objs, for variable span sizes
    // for the first page we have to subtract the full header
    class_to_objs_[i] = (sys_page_sz - sizeof(SlabHeader))/class_to_size_[i];
    // for all following pages we subtract the forward header
    class_to_objs_[i] += (kPageMultiple - 1) *
      ((sys_page_sz - sizeof(ForwardHeader)) / class_to_size_[i]);
    class_to_span_size_[i] = 1UL << 13; //8k
  }
  for (size_t i = kFineClasses; i < kNumClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] * 2; 
    class_to_objs_[i] = 64; // TODO: super dynamic
    class_to_span_size_[i] = ((class_to_objs_[i] * class_to_size_[i] + sizeof(SpanHeader))  / RuntimeVars::SystemPageSize() + 1) * RuntimeVars::SystemPageSize();
  }
}

}  // namespace scalloc

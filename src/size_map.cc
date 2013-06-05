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
    class_to_span_size_[i] = 1UL << 14; //16k
    class_to_objs_[i] = (class_to_span_size_[i] - sizeof(SpanHeader)) / class_to_size_[i];
  }
  for (size_t i = kFineClasses; i < kNumClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] * 2; 
    class_to_objs_[i] = 64; // TODO: super dynamic
    class_to_span_size_[i] = ((class_to_objs_[i] * class_to_size_[i] + sizeof(SpanHeader))  / RuntimeVars::SystemPageSize() + 1) * RuntimeVars::SystemPageSize();
  }
}

}  // namespace scalloc

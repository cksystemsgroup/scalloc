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
  for (size_t i = 1; i < kNumClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] + kMinAlignment;
    const size_t sys_page_sz = RuntimeVars::SystemPageSize();
    // for the first page we have to subtract the full header
    class_to_objs_[i] = (sys_page_sz - sizeof(SlabHeader))/class_to_size_[i];
    // for all following pages we subtract the forward header
    class_to_objs_[i] += (kPageMultiple - 1) *
      ((sys_page_sz - sizeof(ForwardHeader)) / class_to_size_[i]);
  }
}

}  // namespace scalloc 

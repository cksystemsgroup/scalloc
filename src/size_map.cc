#include "size_map.h"

#include "common.h"

namespace scalloc {

void SizeMap::InitModule() {
  SizeMap::Instance().Init();
}

void SizeMap::Init() {
  class_to_size_[0] = 0;
  for (size_t i = 1; i < kNumClasses; i++) {
    class_to_size_[i] = class_to_size_[i-1] + kMinAlignment;
  }
}

}  // namespace scalloc 

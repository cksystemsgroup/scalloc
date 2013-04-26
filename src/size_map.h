#ifndef SCALLOC_SIZE_MAP_H_
#define SCALLOC_SIZE_MAP_H_

#include "common.h"

namespace scalloc {

class SizeMap {
 public:
  static void InitModule();

  static SizeMap& Instance();
  static size_t SizeToClass(const size_t size);

  void Init();
  size_t ClassToSize(const size_t sclass);

 private:
  size_t class_to_size_[kNumClasses];
};

always_inline SizeMap& SizeMap::Instance() {
  static SizeMap singleton;
  return singleton;
}

always_inline size_t SizeMap::SizeToClass(const size_t size) {
  return (size + kMinAlignment - 1) / kMinAlignment;
}

always_inline size_t SizeMap::ClassToSize(const size_t sclass) {
  return class_to_size_[sclass];
}

}  // namespace scalloc

#endif  //SCALLOC_SIZE_MAP_H_

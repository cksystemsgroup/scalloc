// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SIZE_MAP_H_
#define SCALLOC_SIZE_MAP_H_

#include "common.h"
#include "utils.h"

namespace scalloc {

class SizeMap {
 public:
  static void InitModule();

  static SizeMap& Instance();
  static size_t SizeToClass(const size_t size);
  static size_t SizeToBlockSize(const size_t size);

  void Init();
  size_t ClassToSize(const size_t sclass);
  size_t ClassToSpanSize(const size_t sclass);
  size_t MaxObjectsPerClass(const size_t sclass);
  void PrintSizeMap();
  
 private:
  size_t class_to_size_[kNumClasses];
  size_t class_to_objs_[kNumClasses];
  size_t class_to_span_size_[kNumClasses];
};

inline SizeMap& SizeMap::Instance() {
  static SizeMap singleton;
  return singleton;
}

inline size_t SizeMap::SizeToClass(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) / kMinAlignment;
  }
  if (size <= kMaxMediumSize) {
    return utils::Log2(size - 1) - kMaxSmallShift + kFineClasses;
  }
  return 0;
}

inline size_t SizeMap::SizeToBlockSize(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) & ~(kMinAlignment-1);
  } else if (size <= kMaxMediumSize) {
    return 1UL << (utils::Log2(size - 1) + 1);
  }
  // TODO(maigner): never called on large objects. handle anyways
  return 0;
}

inline size_t SizeMap::ClassToSize(const size_t sclass) {
  return class_to_size_[sclass];
}

inline size_t SizeMap::MaxObjectsPerClass(const size_t sclass) {
  return class_to_objs_[sclass];
}

inline size_t SizeMap::ClassToSpanSize(const size_t sclass) {
  return class_to_span_size_[sclass];
}

}  // namespace scalloc

#endif  // SCALLOC_SIZE_MAP_H_

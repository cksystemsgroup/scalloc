#ifndef SCALLOC_SIZE_CLASSES_H_
#define SCALLOC_SIZE_CLASSES_H_

#include "assert.h"
#include "common.h"
#include "utils.h"

namespace scalloc {

extern const uint64_t ClassToObjects[];
extern const uint64_t ClassToSize[];
extern const uint64_t ClassToSpanSize[];

inline size_t SizeToClass(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) / kMinAlignment;
  }
  if (size <= kMaxMediumSize) {
    return utils::Log2(size - 1) - kMaxSmallShift + kFineClasses;
  }
  UNREACHABLE();
  return 0;
}

inline size_t SizeToBlockSize(const size_t size) {
  if (size <= kMaxSmallSize) {
    return (size + kMinAlignment - 1) & ~(kMinAlignment-1);
  } else if (size <= kMaxMediumSize) {
    return 1UL << (utils::Log2(size - 1) + 1);
  }
  UNREACHABLE();
  return 0;
}

  /*
inline void PrintSizeMap() {
  printf("sizeclass summary 2\n");
  printf("kNumClasses: %lu\nkFineClasses: %lu\nkFineClasses/2: %lu\n",
         kNumClasses, kFineClasses, kFineClasses/2);
  for (int i = 0; i < kNumClasses; i++) {
    printf("sc: %d, size: %llu, objs: %llu, real span size: %llu\n",
           i,
           ClassToSize[i],
           ClassToObjects[i],
           ClassToSpanSize[i]);
  }
}
   */
  
}  // namespace scalloc

#endif  // SCALLOC_SIZE_CLASSES_H_
#ifndef SCALLOC_ALLOCATORS_LARGE_OBJECT_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_LARGE_OBJECT_ALLOCATOR_H_

#include <cstddef>

#include "block_header.h"
#include "common.h"
#include "runtime_vars.h"
#include "system-alloc.h"

namespace scalloc {

class LargeObjectAllocator {
 public:
  static void* Alloc(size_t size);
  static void Free(LargeObjectHeader* lbh);
};

inline void* LargeObjectAllocator::Alloc(size_t size) {
  size = PadSize(size, RuntimeVars::SystemPageSize());
  size_t actual_size;
  uintptr_t p = reinterpret_cast<uintptr_t>(scalloc::SystemAlloc_Mmap(size, &actual_size));
  if (UNLIKELY(p % RuntimeVars::SystemPageSize() != 0)) {
    ErrorOut("large malloc alignment failed");
  }
  LargeObjectHeader* lbh = reinterpret_cast<LargeObjectHeader*>(p);
  lbh->Reset(actual_size);
  LOG(kTrace, "large malloc: size: %lu, actual: %lu, block: %p, returned %p",
      size, actual_size, p, reinterpret_cast<void*>(p + sizeof(*lbh)));
  return reinterpret_cast<void*>(p + sizeof(*lbh)); 
};

inline void LargeObjectAllocator::Free(LargeObjectHeader* lbh) {
  scalloc::SystemFree_Mmap(reinterpret_cast<void*>(lbh), lbh->size);
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_LARGE_OBJECT_ALLOCATOR_H_

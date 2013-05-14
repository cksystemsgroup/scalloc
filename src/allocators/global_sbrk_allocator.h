#ifndef SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_

#include <stdint.h>

#include "common.h"
#include "runtime_vars.h"

class GlobalSbrkAllocator {
 public:
  static void InitModule();
  static void* Allocate(const size_t size);

 private:
  static cache_aligned uintptr_t current_;
};

inline void* GlobalSbrkAllocator::Allocate(const size_t size) {
  return reinterpret_cast<void*>(__sync_fetch_and_add(&current_, size));
}

#endif  // SCALLOC_ALLOCATORS_GLOBAL_SBRK_ALLOCATOR_H_

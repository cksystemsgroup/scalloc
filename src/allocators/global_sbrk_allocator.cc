#include "allocators/global_sbrk_allocator.h"

#include <sys/mman.h>  // mmap

#include "log.h"

uintptr_t GlobalSbrkAllocator::current_;

void GlobalSbrkAllocator::InitModule() {
  size_t size = 1UL << 35;  // 32GiB
  void* p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED) {
    ErrorOut("mmap failed");
  }
  current_ = reinterpret_cast<uintptr_t>(p);
}

#include "heap_profiler.h"

#ifdef HEAP_PROFILE

namespace {

  
}  // namespace

namespace scalloc {

TypedAllocator<HeapProfiler>* HeapProfiler::allocator;

void HeapProfiler::Init(TypedAllocator<HeapProfiler>* alloc) {
  allocator = alloc;
}
 
HeapProfiler* HeapProfiler::New() {
  return allocator->New();
}

}

#endif  // HEAP_PROFILE

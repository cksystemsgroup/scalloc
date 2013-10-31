#include "heap_profiler.h"

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
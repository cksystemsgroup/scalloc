#ifndef SCALLOC_HEAP_PROFILER_H_
#define SCALLOC_HEAP_PROFILER_H_

#include "thread_cache.h"
#include "typed_allocator.h"

namespace scalloc {

#ifndef HEAP_PROFILE

  //#define PROFILE_ALLOCATION(ptr, size) do {                                     \
  //} while (0)

#else  // HEAP_PROFILE

  //#define PROFILE_ALLOCATION2(ptr, size) do {                                     \
  //HeapProfiler::Get()->LogAllocation(ptr, size);                               \
  //while (0)

struct ProfileData {

};
  
class HeapProfiler {
 public:
  static void Init(TypedAllocator<HeapProfiler>* alloc);
  static HeapProfiler* Get();
  static HeapProfiler* New();
  
  void LogAllocation(void* ptr, size_t size);
  
 private:
  static TypedAllocator<HeapProfiler>* allocator;
  
  ProfileData data_;
};

  /*
inline HeapProfiler* HeapProfiler::Get() {
  HeapProfiler* profiler = ThreadCache::GetCache().Profiler();
  if (profiler == NULL) {
    profiler = allocator->New();
    memset(&profiler->data_, 0, sizeof(profiler->data_));
    ThreadCache::GetCache().SetProfiler(profiler);
  }
  return profiler;
}
   */

inline void HeapProfiler::LogAllocation(void *ptr, size_t size) {
  LOG(kWarning, "LOGGING ALLOCATION");
}

#endif  // HEAP_PROFILE
  
}  // namespace scalloc

#endif  // SCALLOC_HEAP_PROFILER_H_
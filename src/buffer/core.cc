#include "buffer/core.h"

#include "allocators/scalloc_core-inl.h"
#include "spinlock-inl.h"
#include "typed_allocator.h"
#include "utils.h"

namespace {

scalloc::TypedAllocator<scalloc::CoreBuffer> core_buffer_alloc;
SpinLock new_buffer_lock(LINKER_INITIALIZED);

}

namespace scalloc {

uint64_t CoreBuffer::num_cores_;
pthread_key_t CoreBuffer::core_key;
uint64_t CoreBuffer::thread_counter_;
CoreBuffer* CoreBuffer::buffers_[CoreBuffer::kMaxCores];


void CoreBuffer::Init() {
  num_cores_ = utils::Cpus();
  for (uint64_t i = 0; i < kMaxCores; i++) {
    buffers_[i]  = NULL;
  }
  pthread_key_create(&core_key, NULL);
  thread_counter_ = 0;
  core_buffer_alloc.Init(kPageSize, 64, "core_buffer_alloc");
}


CoreBuffer::CoreBuffer(uint64_t core_id) {
  allocator_ = ScallocCore<LockMode::kSizeClassLocked>::New(core_id);
#ifdef PROFILER
  profiler_.Init(&GlobalProfiler);
#endif  // PROFILER
}


CoreBuffer* CoreBuffer::NewIfNecessary(uint64_t core_id) {
  LockScope(new_buffer_lock);

  if (buffers_[core_id] == NULL) {
    buffers_[core_id] = new(core_buffer_alloc.New()) CoreBuffer(core_id);
  }
  return buffers_[core_id];
}


void CoreBuffer::DestroyBuffers() {
  for (uint64_t i = 0; i < kMaxCores; i++) {
    if (buffers_[i] != NULL) {
      ScallocCore<LockMode::kSizeClassLocked>::Destroy(buffers_[i]->Allocator()); 
#ifdef PROFILER
      buffers_[i]->profiler_.Report();
#endif  // PROFILER
    }
  }
}

}

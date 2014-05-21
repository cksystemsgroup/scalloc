#include "buffer/core.h"

#include "allocators/scalloc_core-inl.h"
#include "common.h"
#include "spinlock-inl.h"
#include "typed_allocator.h"
#include "utils.h"

namespace {

cache_aligned scalloc::TypedAllocator<scalloc::CoreBuffer> core_buffer_alloc;
cache_aligned FastLock new_buffer_lock;

}

namespace scalloc {

uint64_t CoreBuffer::num_cores_;
pthread_key_t CoreBuffer::core_key;
pthread_key_t CoreBuffer::op_key;
uint64_t CoreBuffer::thread_counter_;
uint64_t CoreBuffer::active_threads_;
uint64_t CoreBuffer::active_threads_threshold_;
CoreBuffer* CoreBuffer::buffers_[CoreBuffer::kMaxCores];
uint64_t CoreBuffer::average_sleeping_threads_;


void CoreBuffer::Init() {
  for (uint64_t i = 0; i < kMaxCores; i++) {
    buffers_[i]  = NULL;
  }
  new_buffer_lock.Init();
  num_cores_ = utils::Cpus();
  thread_counter_ = 0;
  active_threads_ = 0;
  active_threads_threshold_ = 0;
  core_buffer_alloc.Init(kPageSize, 64, "core_buffer_alloc");
  pthread_key_create(&core_key, CoreBuffer::ThreadDestructor);
  pthread_key_create(&op_key, NULL);
  average_sleeping_threads_ = 0;
}


CoreBuffer::CoreBuffer(uint64_t core_id) {
  allocator_ = ScallocCore<LockMode::kSizeClassLocked>::New(core_id);
  num_threads_ = 1;
  migratable_ = false;
  sleeping_threads_ = 0;
#ifdef PROFILER
  profiler_.Init(&GlobalProfiler);
#endif  // PROFILER
}


CoreBuffer* CoreBuffer::NewIfNecessary(uint64_t core_id) {
  FastLockScope(new_buffer_lock);

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


void CoreBuffer::ThreadDestructor(void* core_id) {
  __sync_fetch_and_sub(&active_threads_, 1);
#if defined(CLAB_THREADS)
  active_threads_threshold_ = active_threads_ * (kDrift + 100)/num_cores_;
#endif  // CLAG_ACTIVE_THREADS
  CoreBuffer* buffer = buffers_[reinterpret_cast<uint64_t>(core_id) - 1];
  uint64_t old = __sync_fetch_and_sub(&buffer->num_threads_, 1);
  if (old == 1) {
    buffer->ClearSpans(buffer->Allocator());
  }
}


void CoreBuffer::UpdateSleeping() {
  sleeping_threads_ = allocator_->SleepingThreads();
  average_sleeping_threads_ = CalculateAverageSleeping() * (kDrift + 100) / num_cores_;
  migratable_ = average_sleeping_threads_ >= (sleeping_threads_ * 100);
}


void CoreBuffer::ClearSpans(ScallocCore<LockMode::kSizeClassLocked>* allocator) {
  allocator->FreeAllSizeClasses();
}

}

#include "buffer/lab.h"

#include "allocators/scalloc_core-inl.h"
#include "utils.h"

namespace {

cache_aligned scalloc::TypedAllocator<scalloc::AllocationBuffer>
    allocation_buffer_allocator;

}

namespace scalloc {

void AllocationBuffer::ResetAllocator() {
  allocator_->FreeAllSizeClassesLocked();
}


void AllocationBuffer::MakeLocked() {
  allocator_->MakeLocked();
}


void RRAllocationBuffer::Init() {
  allocation_buffer_allocator.Init(
      kPageSize, 64, "allocation_buffer_allocator");
  pthread_key_create(&lab_key_, RRAllocationBuffer::ThreadDestructor);
  thread_counter_ = 0;
  num_labs_ = utils::Cpus();
  for (uint64_t i = 0; i < num_labs_; i++) {
    labs_[i] = allocation_buffer_allocator.New();
    labs_[i]->SetAllocator(ScallocCore::New(i+1));
    labs_[i]->threads = 0;
    labs_[i]->used_ = 0;
  }
  enabled_ = true;
}


void RRAllocationBuffer::ThreadDestructor(void* lab) {
  AllocationBuffer* ab = reinterpret_cast<AllocationBuffer*>(lab);
  MemoryBarrier();
  while(ab->used_ != 0);
  ab->ResetAllocator();
}

}  // namespace scalloc

#include "buffer/core.h"

#include "utils.h"

namespace scalloc {

uint64_t CoreBuffer::num_cores_;
uint64_t CoreBuffer::core_counter_;
SpinLock CoreBuffer::new_allocator_lock_;
SmallAllocator<LockMode::kSizeClassLocked>* CoreBuffer::allocators_[CoreBuffer::kMaxCores];


void CoreBuffer::Init() {
  core_counter_ = 0;
  new_allocator_lock_.Reset();
  num_cores_ = utils::Cpus();
  for (uint64_t i = 0; i < kMaxCores; i++) {
    allocators_[i]  = NULL;
  }
}


SmallAllocator<LockMode::kSizeClassLocked>* CoreBuffer::NewIfNecessary(uint64_t core_id) {
  LockScope(new_allocator_lock_);

  if (allocators_[core_id] == NULL) {
    allocators_[core_id] = SmallAllocator<LockMode::kSizeClassLocked>::New(core_counter_++);
  }
  return allocators_[core_id];
}

}

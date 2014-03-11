// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BUFFER_CORE_H_
#define SCALLOC_BUFFER_CORE_H_

#include "allocators/small_allocator.h"
#include "spinlock-inl.h"
#include "typed_allocator.h"

namespace scalloc {

class CoreBuffer {
 public:
  static const uint64_t kMaxCores = 160;

  static void Init();
  static SmallAllocator<LockMode::kSizeClassLocked>& Allocator();

 private:
  static SmallAllocator<LockMode::kSizeClassLocked>* NewIfNecessary(uint64_t core_id);

  static uint64_t num_cores_;
  static uint64_t core_counter_;
  static SpinLock new_allocator_lock_;
  static SmallAllocator<LockMode::kSizeClassLocked>* allocators_[kMaxCores];
};


inline SmallAllocator<LockMode::kSizeClassLocked>& CoreBuffer::Allocator() {
  uint64_t core_id;
#ifdef __APPLE__
  core_id = hwrand() % num_cores_;
#endif  // __APPLE__
#ifdef __linux__
  core_id = static_cast<uint64_t>(sched_getcpu());
#endif  // __linux__
  SmallAllocator<LockMode::kSizeClassLocked>* alloc = allocators_[core_id];
  if (LIKELY(alloc != NULL)) {
    return *alloc;
  }
  alloc = NewIfNecessary(core_id);
  ScallocAssert(alloc != NULL);
  return *alloc;
}

}  // namespace scalloc

#endif  // SCALLOC_BUFFER_CORE_H_


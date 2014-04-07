// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BUFFER_CORE_H_
#define SCALLOC_BUFFER_CORE_H_

#include <pthread.h>

#include "common.h"
#include "profiler.h"

namespace scalloc {

template<LockMode MODE>
class ScallocCore;

class CoreBuffer {
 public:
  static const uint64_t kMaxCores = 160;

  static void Init();
  static void DestroyBuffers();
  static CoreBuffer& GetBuffer();

  inline ScallocCore<LockMode::kSizeClassLocked>* Allocator() {
    return allocator_;
  }

  PROFILER_GETTER

 private:
  static CoreBuffer* NewIfNecessary(uint64_t core_id);

  explicit CoreBuffer(uint64_t core_id);

  static uint64_t num_cores_;
  static uint64_t thread_counter_;
  static pthread_key_t core_key;
  static CoreBuffer* buffers_[kMaxCores];

  ScallocCore<LockMode::kSizeClassLocked>* allocator_;
  PROFILER_DECL;
};


inline CoreBuffer& CoreBuffer::GetBuffer() {
  uint64_t core_id;
#ifdef __APPLE__
  core_id = hwrand() % num_cores_;
#endif  // __APPLE__
#ifdef __linux__
  core_id = reinterpret_cast<uint64_t>(pthread_getspecific(core_key)) - 1;
  if (core_id == static_cast<uint64_t>(-1)) {
    core_id = __sync_fetch_and_add(&thread_counter_, 1) % num_cores_;
    pthread_setspecific(core_key, (void*)(core_id+1));
  } 
#endif  // __linux__
  CoreBuffer* buffer = buffers_[core_id];
  if (LIKELY(buffer != NULL)) {
    return *buffer;
  }
  buffer = NewIfNecessary(core_id);
  return *buffer;
}

}  // namespace scalloc

#endif  // SCALLOC_BUFFER_CORE_H_


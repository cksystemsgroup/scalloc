// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BUFFER_CORE_H_
#define SCALLOC_BUFFER_CORE_H_

#include <pthread.h>

#include "common.h"
#include "headers.h"
#include "profiler.h"

namespace scalloc {

template<LockMode MODE>
class ScallocCore;

class CoreBuffer {
 public:
  static const uint64_t kMaxCores = 160;
  static const uint64_t kDrift = 300;

  static void Init();
  static void DestroyBuffers();
  static CoreBuffer& GetBuffer(int64_t prefered_core = -1);
  static size_t Id();
  static uint64_t OwnerOf(void* block);

  inline ScallocCore<LockMode::kSizeClassLocked>* Allocator() {
    return allocator_;
  }

  PROFILER_GETTER

 private:
  static CoreBuffer* NewIfNecessary(uint64_t core_id);
  static void ThreadDestructor(void* core_buffer);

  explicit CoreBuffer(uint64_t core_id);

  static uint64_t num_cores_;
  static uint64_t thread_counter_;
  static uint64_t active_threads_;
  static pthread_key_t core_key;
  static CoreBuffer* buffers_[kMaxCores];

  ScallocCore<LockMode::kSizeClassLocked>* allocator_;
  uint64_t num_threads_;
  PROFILER_DECL;
};


inline CoreBuffer& CoreBuffer::GetBuffer(int64_t prefered_core) {
  uint64_t core_id = reinterpret_cast<uint64_t>(pthread_getspecific(core_key)) - 1;
  if (core_id == static_cast<uint64_t>(-1)) {
    // New thread joins the system.
    __sync_fetch_and_add(&active_threads_, 1);
#ifdef DYNAMIC_CLAB
    if ((prefered_core != -1) &&
        ((active_threads_*(100+kDrift)/100/num_cores_) > buffers_[prefered_core]->num_threads_)) {
      core_id = prefered_core;
      __sync_fetch_and_sub(&buffers_[core_id]->num_threads_, 1);
    } else {
#endif  // DYNAMIC_CLAB
      core_id = __sync_fetch_and_add(&thread_counter_, 1) % num_cores_;
#ifdef DYNAMIC_CLAB
    }
#endif  // DYNAMIC_CLAB
    pthread_setspecific(core_key, (void*)(core_id+1));
  } else {
#ifdef DYNAMIC_CLAB
    if (prefered_core != -1 && static_cast<uint64_t>(prefered_core) != core_id) {
      if ((active_threads_*(100+kDrift)/100/num_cores_) > buffers_[prefered_core]->num_threads_) {
        core_id = prefered_core;
        pthread_setspecific(core_key, (void*)(core_id+1));
      } 
    }
#endif  // DYNAMIC_CLAB
  }

  CoreBuffer* buffer = buffers_[core_id];
  if (LIKELY(buffer != NULL)) {
    return *buffer;
  }
  buffer = NewIfNecessary(core_id);
  return *buffer;
}


inline size_t CoreBuffer::Id() {
  uint64_t core_id = reinterpret_cast<uint64_t>(pthread_getspecific(core_key)) - 1;
  if (core_id == static_cast<uint64_t>(-1)) {
    return static_cast<size_t>(-1);
  }
  return static_cast<size_t>(core_id);
}


inline uint64_t CoreBuffer::OwnerOf(void* block) {
  return SpanHeader::GetFromObject(block)->aowner.owner;
}

}  // namespace scalloc

#endif  // SCALLOC_BUFFER_CORE_H_


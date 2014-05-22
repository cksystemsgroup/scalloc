// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BUFFER_CORE_H_
#define SCALLOC_BUFFER_CORE_H_

#include <pthread.h>

#include "common.h"
#include "headers.h"
#include "profiler.h"
#include "random.h"

#ifndef CLAB_THRESHOLD
#define CLAB_THRESHOLD 200
#endif  // CLAB_THRESHOLD

namespace scalloc {

template<LockMode MODE>
class ScallocCore;

class CoreBuffer {
 public:
  static const uint64_t kMaxCores = 160;
  static const uint64_t kDrift = CLAB_THRESHOLD;

  static void Init();
  static void DestroyBuffers();
  static CoreBuffer& GetBuffer(int64_t prefered_core = -1);
  static size_t Id();
  static uint64_t OwnerOf(void* block);
  static uint64_t CalculateAverageSleeping();

  inline ScallocCore<LockMode::kSizeClassLocked>* Allocator() {
#if defined(CLAB_UTILIZATION)
    const uint64_t op_cnt = reinterpret_cast<uint64_t>(
        pthread_getspecific(op_key));
    if (op_cnt % (kNumClasses * num_cores_ * active_threads_) == 0) {
      UpdateSleeping();
    }
    pthread_setspecific(op_key, reinterpret_cast<void*>(op_cnt+1));
#endif  // CLAB_UTILIZATION
    return allocator_;
  }

  void UpdateSleeping();

  PROFILER_GETTER

 private:
  static CoreBuffer* NewIfNecessary(uint64_t core_id);
  static void ThreadDestructor(void* core_buffer);

  explicit CoreBuffer(uint64_t core_id);
  void ClearSpans(ScallocCore<LockMode::kSizeClassLocked>* allocator);

  static uint64_t num_cores_;
  static uint64_t thread_counter_;
  static uint64_t active_threads_;
  static uint64_t active_threads_threshold_;
  static pthread_key_t core_key;
  static pthread_key_t op_key;
  static CoreBuffer* buffers_[kMaxCores];
  static uint64_t average_sleeping_threads_;

  ScallocCore<LockMode::kSizeClassLocked>* allocator_;

  uint64_t num_threads_;
  uint64_t sleeping_threads_;
  bool migratable_;

  PROFILER_DECL;
};


inline CoreBuffer& CoreBuffer::GetBuffer(int64_t prefered_core) {
  bool add_cnt = false;
  int64_t remove_from = -1;
  uint64_t core_id = reinterpret_cast<uint64_t>(
      pthread_getspecific(core_key)) - 1;
  if (core_id == static_cast<uint64_t>(-1)) {
    // New thread joins the system.
    __sync_fetch_and_add(&active_threads_, 1);
    add_cnt = true;

#if defined(CLAB_UTILIZATION)
    if (prefered_core != -1 && buffers_[prefered_core]->migratable_) {
      core_id = prefered_core;
    } else {
      core_id = __sync_fetch_and_add(&thread_counter_, 1) % num_cores_;
    }
#elif defined(CLAB_THREADS)
    active_threads_threshold_ = active_threads_ * (kDrift + 100)/num_cores_;
    if ((prefered_core != -1) &&
        (active_threads_threshold_ >= (100 *buffers_[prefered_core]->num_threads_))) {
      core_id = prefered_core;
    } else {
      core_id = __sync_fetch_and_add(&thread_counter_, 1) % num_cores_;
    }
#elif defined(CLAB_RR)
    core_id = __sync_fetch_and_add(&thread_counter_, 1) % num_cores_;
#else
#error "unknown clab assignment policy"
#endif

    pthread_setspecific(core_key, (void*)(core_id+1));
  } else {
#if defined(CLAB_UTILIZATION)
    if (prefered_core != -1 && static_cast<uint64_t>(prefered_core) != core_id) {
      if (buffers_[prefered_core]->migratable_) {
        add_cnt = true;
        remove_from = static_cast<int64_t>(core_id);
        core_id = prefered_core;
        pthread_setspecific(core_key, (void*)(core_id+1));
      }
    }
#elif defined(CLAB_THREADS)
    if (prefered_core != -1 && static_cast<uint64_t>(prefered_core) != core_id) {
      if (active_threads_threshold_ >= (100*buffers_[prefered_core]->num_threads_)) {
        add_cnt = true;
        remove_from = static_cast<int64_t>(core_id);
        core_id = prefered_core;
        pthread_setspecific(core_key, reinterpret_cast<void*>(core_id+1));
      }
    }
#elif defined(CLAB_RR)
    // Nop.
#endif  // DYNAMIC_CLAB
  }

  CoreBuffer* buffer = buffers_[core_id];
  if (LIKELY(buffer != NULL)) {
    if (add_cnt) {
      __sync_fetch_and_add(&buffer->num_threads_, 1);
      if (remove_from != -1) {
        uint64_t old = __sync_fetch_and_sub(
            &buffers_[remove_from]->num_threads_, 1);
        if (old == 1) {
          buffers_[remove_from]->ClearSpans(buffers_[remove_from]->Allocator());
        }
      }
    }
    return *buffer;
  }
  buffer = NewIfNecessary(core_id);
  return *buffer;
}


inline size_t CoreBuffer::Id() {
  uint64_t core_id = reinterpret_cast<uint64_t>(
      pthread_getspecific(core_key)) - 1;
  if (core_id == static_cast<uint64_t>(-1)) {
    return static_cast<size_t>(-1);
  }
  return static_cast<size_t>(core_id);
}


inline uint64_t CoreBuffer::OwnerOf(void* block) {
  return SpanHeader::GetFromObject(block)->aowner.owner;
}


inline uint64_t CoreBuffer::CalculateAverageSleeping() {
  uint64_t sum = 0;
  for (uint64_t i = 0; i < num_cores_; i++) {
    if (buffers_[i] != NULL) {
      sum += buffers_[i]->sleeping_threads_;
    }
  }
  return sum;
}

}  // namespace scalloc

#endif  // SCALLOC_BUFFER_CORE_H_


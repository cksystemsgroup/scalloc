// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_THREAD_CACHE_H_
#define SCALLOC_THREAD_CACHE_H_

#include <pthread.h>

#include "common.h"
#include "headers.h"
#include "profiler.h"

namespace scalloc {

template<LockMode MODE>
class ScallocCore;

// A threadlocal cache that may hold any allocator.  We use __thread as fast
// path, but still implement a pthread_{get|set}specific version, since we need
// a finalizer for threads that terminate and support platforms which do not
// have __thread, or implement it with malloc().
class ThreadCache {
 public:
  static void Init();
  static ThreadCache& GetCache();
  static void DestroyRemainingCaches();

  inline ScallocCore<LockMode::kLocal>* Allocator() { return alloc_; }

  PROFILER_GETTER

 private:
  static void DestroyThreadCache(void* p);
  static ThreadCache* RawGetCache();
  static ThreadCache* NewIfNecessary();
  static ThreadCache* New(pthread_t owner);

#ifdef HAVE_TLS
  // Fast path thread-local access point.
  static __thread ThreadCache* tl_cache_ TLS_MODE;
#endif  // HAVE_TLS
  static bool module_init_;
  static ThreadCache* thread_caches_;
  static pthread_key_t cache_key_;

  ThreadCache(ScallocCore<LockMode::kLocal>* allocator,
              pthread_t owner,
              ThreadCache* next);

  // Actual thread-local data.
  ScallocCore<LockMode::kLocal>* alloc_;
  PROFILER_DECL;

  // House-keeping data.
  bool in_setspecific_;
  pthread_t owner_;
  ThreadCache* next_;
};


inline ThreadCache* ThreadCache::RawGetCache() {
#ifdef HAVE_TLS
  return tl_cache_;
#endif  // HAVE_TLS
  return static_cast<ThreadCache*>(pthread_getspecific(cache_key_));
}


inline ThreadCache& ThreadCache::GetCache() {
  ThreadCache* cache = RawGetCache();
  if (LIKELY(cache != NULL)) {
    return *cache;
  }
  if (!module_init_) {
    Init();
  }
  cache = NewIfNecessary();
  ScallocAssert(cache != NULL);
  return *cache;
}

}  // namespace scalloc

#endif  // SCALLOC_THREAD_CACHE_H_

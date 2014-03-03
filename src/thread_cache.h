// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_THREAD_CACHE_H_
#define SCALLOC_THREAD_CACHE_H_

#include <pthread.h>

#include "common.h"
#include "headers.h"

namespace scalloc {

class SmallAllocator;

// Thread-local policy (POLICY_THREAD_LOCAL):
// We use __thread as fast path, but still implement a pthread_{get|set}specific
// version, since we need a finalizer for threads that terminate and support
// platforms which do not have __thread, or implement it with malloc().
class ThreadCache {
 public:
  static void Init();
  static ThreadCache& GetCache();

  inline SmallAllocator* Allocator() { return alloc_; }

 private:
  static ThreadCache* RawGetCache();
  static ThreadCache* NewIfNecessary();
  static ThreadCache* New(pthread_t owner);
  static void DestroyThreadCache(void* p);

#ifdef HAVE_TLS
  // Fast path thread-local access point.
  static __thread ThreadCache* tl_cache_ TLS_MODE;
#endif  // HAVE_TLS
  static bool module_init_;
  static ThreadCache* thread_caches_;
  static pthread_key_t cache_key_;

  bool in_setspecific_;
  pthread_t owner_;
  ThreadCache* next_;

  SmallAllocator* alloc_;
} cache_aligned;


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

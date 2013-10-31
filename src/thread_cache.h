// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_THREAD_CACHE_H_
#define SCALLOC_THREAD_CACHE_H_

#include <pthread.h>

//#include "allocators/small_allocator.h"
#include "block_header.h"
#include "common.h"

namespace scalloc {

#ifdef HEAP_PROFILE
class HeapProfiler;
#endif  // HEAP_PROFILE
class SmallAllocator;
  
// A threadlocal cache that may hold any allocator.  We use __thread as fast
// path, but still implement a pthread_{get|set}specific version, since we need
// a finalizer for threads that terminate and support platforms which do not
// have __thread, or implement it with malloc().
class ThreadCache {
 public:
  static void InitModule();
  static ThreadCache& GetCache();

//void* Allocate(const size_t size);
//void Free(void* ptr, Header* hdr);
#ifdef HEAP_PROFILE
  inline HeapProfiler* Profiler() { return profiler_; }
#endif  // HEAP_PROFILE
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
  
//  SmallAllocator allocator_;
  SmallAllocator* alloc_;
  bool in_setspecific_;
  pthread_t owner_;
  ThreadCache* next_;
#ifdef HEAP_PROFILE
  HeapProfiler* profiler_;
#endif  // HEAP_PROFILE
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
    InitModule();
  }
  cache = NewIfNecessary();
  ScallocAssert(cache != NULL);
  return *cache;
}

//inline void* ThreadCache::Allocate(const size_t size) {
//  return allocator_.Allocate(size);
//}
//
//inline void ThreadCache::Free(void* p, Header* hdr) {
//  allocator_.Free(p, reinterpret_cast<SpanHeader*>(hdr));
//}

}  // namespace scalloc

#endif  // SCALLOC_THREAD_CACHE_H_

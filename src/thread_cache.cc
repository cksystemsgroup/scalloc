// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/small_allocator.h"
#include "spinlock-inl.h"
#include "thread_cache.h"
#include "typed_allocator.h"

namespace {

cache_aligned SpinLock g_threadcache_lock(LINKER_INITIALIZED);
cache_aligned scalloc::TypedAllocator<scalloc::ThreadCache>
    g_threadcache_alloc;
cache_aligned uint64_t g_thread_id;

}  // namespace

namespace scalloc {

bool ThreadCache::module_init_;
ThreadCache* ThreadCache::thread_caches_ = NULL;
pthread_key_t ThreadCache::cache_key_;
#ifdef HAVE_TLS
__thread TLS_MODE ThreadCache* ThreadCache::tl_cache_;
#endif  // HAVE_TLS

void ThreadCache::InitModule() {
  LockScope(g_threadcache_lock);

  if (!module_init_) {
    g_threadcache_alloc.Init(kPageSize, 64);
    // http://pubs.opengroup.org/onlinepubs/009696799/functions/pthread_key_create.html
    //
    // At thread exit, if a key value has a non-NULL destructor pointer, and the
    // thread has a non-NULL value associated with that key, the value of the
    // key is set to NULL, and then the function pointed to is called with the
    // previously associated value as its sole argument. The order of destructor
    // calls is unspecified if more than one destructor exists for a thread when
    // it exits.
    pthread_key_create(&cache_key_, DestroyThreadCache);
    module_init_  = true;
  }
}

ThreadCache* ThreadCache::New(pthread_t owner) {
  ThreadCache* cache = g_threadcache_alloc.New();
  const uint64_t id = __sync_fetch_and_add(&g_thread_id, 1);
  cache->alloc_ = SmallAllocator::New(id);
#ifdef HEAP_PROFILE
  cache->profiler_ = HeapProfiler::New();
#endif  // HEAP_PROFILE
  cache->owner_ = owner;
  cache->next_ = thread_caches_;
  thread_caches_ = cache;
  LOG(kTrace, "[ThreadCache] creating new cache; id: %lu, p: %p\n", id, cache);
  return cache;
}

ThreadCache* ThreadCache::NewIfNecessary() {
  // This early call may crash on old platforms. We don't care.
  const pthread_t me = pthread_self();
  ThreadCache* cache = NULL;
  {
    LockScope(g_threadcache_lock);

    // pthread_setspecific may call into malloc.
    for (ThreadCache* c = thread_caches_; c != NULL; c = c->next_) {
      if (c->owner_ == me) {
        cache = c;
        break;
      }
    }

    if (cache == NULL) {
      cache = ThreadCache::New(me);
    }
  }

  if (!cache->in_setspecific_) {
    cache->in_setspecific_ = true;
#ifdef HAVE_TLS
    tl_cache_ = cache;
#endif  // HAVE_TLS
    pthread_setspecific(cache_key_, static_cast<void*>(cache));
    cache->in_setspecific_ = false;
  }

  return cache;
}

void ThreadCache::DestroyThreadCache(void* p) {
  if (p == NULL) {
    return;
  }
  LOG(kTrace, "[ThreadCache] destroy; p :%p", p);
#ifdef HAVE_TLS
  tl_cache_ = NULL;
#endif  // HAVE_TLS

  ThreadCache* cache = reinterpret_cast<ThreadCache*>(p);
  cache->Allocator()->Destroy();
//  cache->allocator_.Destroy();

  {
    LockScope(g_threadcache_lock);

    ThreadCache* prev = NULL;
    for (ThreadCache* c = thread_caches_; c != NULL; c = c->next_) {
      if (c == cache) {
        if (prev == NULL) {
          thread_caches_ = NULL;
        } else {
          prev->next_ = c->next_;
        }
        break;
      }
      prev = c;
    }
  }

  // Finally, delete the cache.
  g_threadcache_alloc.Delete(cache);
}

}  // namespace scalloc

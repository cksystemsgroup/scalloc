#include "thread_cache.h"

#include "allocators/dq_sc_allocator.h"
#include "runtime_vars.h"

namespace {

cache_aligned SpinLock g_threadcache_lock(LINKER_INITIALIZED);
cache_aligned scalloc::PageHeapAllocator<scalloc::ThreadCache, 64> g_threadcache_alloc;
cache_aligned uint64_t g_thread_id;

}  // namespace

namespace scalloc {

bool ThreadCache::module_init_;
ThreadCache* ThreadCache::thread_caches_;
pthread_key_t ThreadCache::cache_key_;
__thread TLS_MODE ThreadCache* ThreadCache::tl_cache_;

void ThreadCache::InitModule() {
  SpinLockHolder holder(&g_threadcache_lock);
  CompilerBarrier();
  if(!module_init_) {
    g_threadcache_alloc.Init(RuntimeVars::SystemPageSize());
    module_init_  = true;
    // http://pubs.opengroup.org/onlinepubs/009696799/functions/pthread_key_create.html
    //
    // At thread exit, if a key value has a non-NULL destructor pointer, and the
    // thread has a non-NULL value associated with that key, the value of the key is
    // set to NULL, and then the function pointed to is called with the previously
    // associated value as its sole argument. The order of destructor calls is
    // unspecified if more than one destructor exists for a thread when it exits.
    pthread_key_create(&cache_key_, DestroyThreadCache);
  }
}

ThreadCache* ThreadCache::NewCache() {
  ThreadCache* cache = g_threadcache_alloc.New();
  cache = g_threadcache_alloc.New();
  cache->allocator_.Init(__sync_fetch_and_add(&g_thread_id, 1));

  // pthread_setspecific() may call malloc() itself, thus we MUST set
  // tl_cache_ beforehand to allow a recursive malloc() call to find the cache
  // through the __thread mechanism.
  tl_cache_ = cache;

  // We MUST set the cache_key_, since otherwise the destructor() function is
  // not called.
  pthread_setspecific(cache_key_, cache);
  return cache;
}

void ThreadCache::DestroyThreadCache(void* p) {
  if (p == NULL) {
    return;
  }
  tl_cache_ = NULL;

  ThreadCache* cache = reinterpret_cast<ThreadCache*>(p);
  cache->allocator_.Destroy();

  // Finally, delete the cache.
  g_threadcache_alloc.Delete(cache);
}

}  // namespace scalloc

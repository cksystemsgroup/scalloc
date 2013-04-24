#include "thread_cache.h"

#include "allocators/dq_sc_allocator.h"
#include "runtime_vars.h"

namespace {

cache_aligned SpinLock g_threadcache_lock(LINKER_INITIALIZED);
cache_aligned scalloc::PageHeapAllocator<scalloc::ThreadCache, 64> g_threadcache_alloc;
cache_aligned uint64_t g_thread_id;

}  // namespace

namespace scalloc {

void ThreadCache::InitModule() {
  SpinLockHolder holder(&g_threadcache_lock);
  CompilerBarrier();
  if(!module_init_) {
    RuntimeVars::InitModule();
    SlabScAllocator::InitModule();
    DQScAllocator::InitModule();

    g_threadcache_alloc.Init(RuntimeVars::SystemPageSize());

    module_init_  = true;
  }
}

ThreadCache* ThreadCache::NewCache() {
  ThreadCache* cache = NULL;
  {
    SpinLockHolder holder(&g_threadcache_lock);
    CompilerBarrier();
    cache = g_threadcache_alloc.New();
  }
  cache->allocator_.Init(__sync_fetch_and_add(&g_thread_id, 1));
  return cache;
}

}  // namespace scalloc

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "scalloc.h"

#include <errno.h>
#include <string.h>

#include "allocators/block_pool.h"
#include "allocators/large_allocator.h"
#include "allocators/small_allocator.h"
#include "allocators/span_pool.h"
#include "common.h"
#include "distributed_queue.h"
#include "override.h"
#include "scalloc_arenas.h"
#include "scalloc_guard.h"
#include "size_map.h"
#include "thread_cache.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

#ifndef __THROW
#define __THROW
#endif

namespace scalloc {

cache_aligned Arena InternalArena;

cache_aligned Arena SmallArena;

}  // namespace scalloc

static int scallocguard_refcount = 0;
ScallocGuard::ScallocGuard() {
  if (scallocguard_refcount++ == 0) {
    ReplaceSystemAlloc();

    scalloc::SizeMap::InitModule();
    scalloc::InternalArena.Init(kInternalSpace);
    scalloc::SmallArena.Init(kSmallSpace);
    DistributedQueue::InitModule();
    scalloc::SpanPool::InitModule();
    scalloc::BlockPool::InitModule();
    scalloc::SmallAllocator::InitModule();
    scalloc::ThreadCache::InitModule();

    free(malloc(1));

#ifdef PROFILER_ON
    scalloc::GlobalProfiler::Instance().Init();
    scalloc::Profiler::Enable();
#endif  // PROFILER_ON
  }
}

ScallocGuard::~ScallocGuard() {
  if (--scallocguard_refcount == 0) {
  }
}

static ScallocGuard StartupExitHook;

namespace scalloc {

void* malloc(const size_t size) {
  void* p;
  if (LIKELY(size <= kMaxMediumSize && SmallAllocator::Enabled())) {
    p = ThreadCache::GetCache().Allocate(size);
  } else {
    p = LargeAllocator::Alloc(size);
  }
  if (UNLIKELY(p == NULL)) {
    errno = ENOMEM;
  }
  return p;
}

void free(void* p) {
  if (UNLIKELY(p == NULL)) {
    return;
  }
  if (SmallArena.Contains(p)) {
    ThreadCache::GetCache().Free(p, reinterpret_cast<SpanHeader*>(
        SpanHeader::GetFromObject(p)));
  } else {
    LargeAllocator::Free(reinterpret_cast<LargeObjectHeader*>(
        LargeObjectHeader::GetFromObject(p)));
  }
  return;
}

void* calloc(size_t nmemb, size_t size) {
  const size_t malloc_size = nmemb * size;
  if (size != 0 && (malloc_size / size) != nmemb) {
    return NULL;
  }
  void* result = malloc(malloc_size);  // also sets errno
  if (result != NULL) {
    memset(result, 0, malloc_size);
  }
  return result;
}

void* realloc(void* ptr, size_t size) {
  if (ptr == NULL) {
    return malloc(size);
  }
  void* new_ptr;
  size_t old_size;
  if (scalloc::SmallArena.Contains(ptr)) {
    old_size = SizeMap::Instance().ClassToSize(
        reinterpret_cast<SpanHeader*>(
            SpanHeader::GetFromObject(ptr))->size_class);
  } else {
    old_size =
        reinterpret_cast<LargeObjectHeader*>(
            LargeObjectHeader::GetFromObject(ptr))->size
        - sizeof(LargeObjectHeader);
  }
  if (size <= old_size) {
    return ptr;
  } else {
    new_ptr = malloc(size);
    if (LIKELY(new_ptr != NULL)) {
      memcpy(new_ptr, ptr, old_size);
      free(ptr);
    }
  }
  return new_ptr;
}

void* memalign(size_t __alignment, size_t __size) {
  ErrorOut("memalign() not yet implemented.");
}

int posix_memalign(void** ptr, size_t align, size_t size) {
  ErrorOut("posix_memalign() not yet implemented.");
}

void* valloc(size_t __size) {
  ErrorOut("valloc() not yet implemented.");
}

void* pvalloc(size_t __size) {
  ErrorOut("pvalloc() not yet implemented.");
}

void malloc_stats(void) {
  ErrorOut("malloc_stats() not yet implemented.");
}

int mallopt(int cmd, int value) {
  ErrorOut("mallopt() not yet implemented.");
}

bool Ours(const void* p) {
  return SmallArena.Contains(const_cast<void*>(p)) || LargeAllocator::Owns(p);
}

}  // namespace scalloc

extern "C" {
void* scalloc_malloc(size_t size) __THROW {
  return scalloc::malloc(size);
}

void scalloc_free(void* p) __THROW {
  scalloc::free(p);
}

void* scalloc_calloc(size_t nmemb, size_t size) __THROW {
  return scalloc::calloc(nmemb, size);
}

void* scalloc_realloc(void* ptr, size_t size) __THROW {
  return scalloc::realloc(ptr, size);
}

void* scalloc_memalign(size_t __alignment, size_t __size) __THROW {
  return scalloc::memalign(__alignment, __size);
}

int scalloc_posix_memalign(void** ptr, size_t align, size_t size) __THROW {
  return scalloc::posix_memalign(ptr, align, size);
}

void* scalloc_valloc(size_t __size) __THROW {
  return scalloc::valloc(__size);
}

void* scalloc_pvalloc(size_t __size) __THROW {
  return scalloc::pvalloc(__size);
}

void scalloc_malloc_stats() __THROW {
  scalloc::malloc_stats();
}

int scalloc_mallopt(int cmd, int value) __THROW {
  return scalloc::mallopt(cmd, value);
}
}

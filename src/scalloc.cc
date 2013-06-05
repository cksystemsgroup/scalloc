// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "scalloc.h"

#include <errno.h>
#include <string.h>

#include "allocators/block_pool.h"
#include "allocators/large_allocator.h"
#include "allocators/medium_allocator.h"
#include "allocators/small_allocator.h"
#include "allocators/span_pool.h"
#include "common.h"
#include "distributed_queue.h"
#include "runtime_vars.h"
#include "scalloc_arenas.h"
#include "scalloc_guard.h"
#include "size_map.h"
#include "thread_cache.h"

#ifdef PROFILER_ON
#include "profiler.h"
scalloc::Profiler global_profiler;
#endif  // PROFILER_ON

Arena SmallArea;

static int scallocguard_refcount = 0;
ScallocGuard::ScallocGuard() {
  if (scallocguard_refcount++ == 0) {
    RuntimeVars::InitModule();
    scalloc::SizeMap::InitModule();

    scalloc::InitArenas();

    DistributedQueue::InitModule();

    scalloc::SpanPool::InitModule();
    scalloc::BlockPool::InitModule();

    scalloc::SmallAllocator::InitModule();
    scalloc::MediumAllocator::InitModule();

    free(malloc(1));
#ifdef PROFILER_ON
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

always_inline void* malloc(const size_t size) {
  void* p;
  if (LIKELY(size <= kMaxMediumSize && SmallAllocator::Enabled())) {
    p = ThreadCache::GetCache().Allocate(size);
  /*
  } else if (size < kMaxMediumSize && MediumAllocator::Enabled()) {
    p = MediumAllocator::Allocate(size);
  */
  } else {
    p = LargeAllocator::Alloc(size);
  }
  if (UNLIKELY(p == NULL)) {
    errno = ENOMEM;
  }
  return p;
}

always_inline void free(void* p) {
  if (UNLIKELY(p == NULL)) {
    return;
  }
  if (SmallArena.Contains(p)) {
    ThreadCache::GetCache().Free(p, reinterpret_cast<SpanHeader*>(
        SpanHeader::GetFromObject(p)));
  /*
  } else if (MediumArena.Contains(p)) {
    MediumAllocator::Free(p);
  */
  } else {
    LargeAllocator::Free(reinterpret_cast<LargeObjectHeader*>(
        BlockHeader::GetFromObject(p)));
  }
  return;
}

}  // namespace scalloc


//
// Forward C calls to scalloc_fn to their corresponding scalloc::fn
//

extern "C" void* scalloc_malloc(size_t size) __THROW {
  return scalloc::malloc(size);
}

extern "C" void scalloc_free(void* p) __THROW {
  scalloc::free(p);
}

extern "C" void* scalloc_calloc(size_t nmemb, size_t size) __THROW {
  const size_t malloc_size = nmemb * size;
  if (size != 0 && (malloc_size / size) != nmemb) {
    return NULL;
  }
  void* result = scalloc::malloc(malloc_size);  // also sets errno
  if (result != NULL) {
    memset(result, 0, malloc_size);
  }
  return result;
}

extern "C" void* scalloc_realloc(void* ptr, size_t size) __THROW {
  if (ptr == NULL) {
    return scalloc::malloc(size);
  }
  void* new_ptr;
  size_t old_size;
  if (scalloc::SmallArena.Contains(ptr)) {
    old_size = scalloc::SizeMap::Instance().ClassToSize(
        reinterpret_cast<SpanHeader*>(SpanHeader::GetFromObject(ptr))->size_class);
  /*
  } else if (scalloc::MediumArena.Contains(ptr)) {
    old_size = scalloc::MediumAllocator::SizeOf(ptr); 
  */
  } else {
    old_size = reinterpret_cast<LargeObjectHeader*>(BlockHeader::GetFromObject(ptr))->size -
               sizeof(LargeObjectHeader);
  }
  if (size <= old_size) {
    return ptr;
  } else {
    new_ptr = scalloc::malloc(size);
    if (LIKELY(new_ptr != NULL)) {
      memcpy(new_ptr, ptr, old_size);
      scalloc::free(ptr);
    }
  }
  return new_ptr;
}

extern "C" void* scalloc_memalign(size_t __alignment, size_t __size) __THROW {
  ErrorOut("memalign() not yet implemented.");
}

extern "C" int scalloc_posix_memalign(
    void** ptr, size_t align, size_t size) __THROW {
  ErrorOut("posix_memalign() not yet implemented.");
}

extern "C" void* scalloc_valloc(size_t __size) __THROW {
  ErrorOut("valloc() not yet implemented.");
}

extern "C" void* scalloc_pvalloc(size_t __size) __THROW {
  ErrorOut("pvalloc() not yet implemented.");
}

extern "C" void scalloc_malloc_stats(void) __THROW {
  ErrorOut("malloc_stats() not yet implemented.");
}

extern "C" int scalloc_mallopt(int cmd, int value) __THROW {
  ErrorOut("mallopt() not yet implemented.");
}

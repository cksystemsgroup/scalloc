// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "scalloc.h"

#include <errno.h>
#include <string.h>

#include "allocators/block_pool.h"
#include "allocators/large_allocator.h"
#include "allocators/small_allocator.h"
#include "allocators/span_pool.h"
#include "assert.h"
#include "collector.h"
#include "common.h"
#include "distributed_queue.h"
#include "list-inl.h"
#include "override.h"
#include "random.h"
#include "scalloc_arenas.h"
#include "scalloc_guard.h"
#include "size_classes_raw.h"
#include "size_classes.h"
#include "thread_cache.h"
#include "utils.h"

#ifdef PROFILER_ON
#include "profiler.h"
#endif  // PROFILER_ON

namespace scalloc {

cache_aligned Arena InternalArena;
cache_aligned Arena SmallArena;

cache_aligned const uint64_t ClassToObjects[] = {
#define SIZE_CLASS(a, b, c, d) (d),
SIZE_CLASSES
#undef SIZE_CLASS
};

cache_aligned const uint64_t ClassToSize[] = {
#define SIZE_CLASS(a, b, c, d) (b),
SIZE_CLASSES
#undef SIZE_CLASS
};

cache_aligned const uint64_t ClassToSpanSize[] = {
#define SIZE_CLASS(a, b, c, d) (c),
SIZE_CLASSES
#undef SIZE_CLASS
};

// Allocators for internal data structures.
#ifdef HEAP_PROFILE
cache_aligned TypedAllocator<HeapProfiler> profile_allocator;
#endif  // HEAP_PROFILE
cache_aligned TypedAllocator<SmallAllocator<LockMode::kLocal>>
    small_allocator_allocator;
cache_aligned TypedAllocator<DistributedQueue::State> dq_state_allocator;
cache_aligned TypedAllocator<DistributedQueue::Backend> dq_backend_allocator;


inline void CheckSizeClasses() {
  uint64_t payload;
  for (size_t i = 1; i < kNumClasses; i++) {
    payload = ClassToSize[i] * ClassToObjects[i];
    if ((payload + sizeof(SpanHeader)) > ClassToSpanSize[i]) {
      LOG(kError, "size class: %lu, size: %lu, objects: %lu, span size: %lu",
          i, ClassToSize[i], ClassToObjects[i], ClassToSpanSize[i]);
      Fatal("inconsistent size classes");
    }
  }
}

}  // namespace scalloc

static int scallocguard_refcount = 0;
ScallocGuard::ScallocGuard() {
  if (scallocguard_refcount++ == 0) {
#ifdef DEBUG
    SpanHeader::CheckFieldAlignments();
    LargeObjectHeader::CheckFieldAlignments();
    scalloc::CheckSizeClasses();
#endif  // DEBUG

    ReplaceSystemAlloc();

    scalloc::InternalArena.Init(kInternalSpace);
    scalloc::SmallArena.Init(kSmallSpace);

    scalloc::dq_state_allocator.Init(kPageSize * 16, 64, "dq_state_allocator");
    scalloc::dq_backend_allocator.Init(kPageSize * 16, 64, "dq_backend_allocator");
    scalloc::DistributedQueue::Init(&scalloc::dq_state_allocator,
                                    &scalloc::dq_backend_allocator);
    scalloc::SpanPool::Init();
    scalloc::BlockPool::Init();

#ifdef COLLECTOR
    scalloc::Collector::Init();
#endif  // COLLECTOR

    scalloc::small_allocator_allocator.Init(kPageSize * 4, 64, "small_allocator_allocator");
    scalloc::SmallAllocator<scalloc::LockMode::kLocal>::Init(&scalloc::small_allocator_allocator);

    scalloc::ThreadCache::Init();

#ifdef HEAP_PROFILE
    scalloc::profile_allocator.Init(kPageSize, 64);
    scalloc::HeapProfiler::Init(&scalloc::profile_allocator);
#endif  // HEAP_PROFILE

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
  LOG(kTrace, "malloc: size: %lu", size);
  void* p;
  if (LIKELY(size <= kMaxMediumSize && SmallAllocator<LockMode::kLocal>::Enabled())) {
    p = ThreadCache::GetCache().Allocator()->Allocate(size);
  } else {
    p = LargeAllocator::Alloc(size);
  }
  if (UNLIKELY(p == NULL)) {
    errno = ENOMEM;
  }
  LOG(kTrace, "malloc: returning %p", p);
  return p;
}


void free(void* p) {
  if (UNLIKELY(p == NULL)) {
    return;
  }
  LOG(kTrace, "free: %p", p);
  if (LIKELY(SmallArena.Contains(p))) {
    ThreadCache::GetCache().Allocator()->Free(p, SpanHeader::GetFromObject(p));
  } else {
    LargeAllocator::Free(LargeObjectHeader::GetFromObject(p));
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
    old_size = ClassToSize[SpanHeader::GetFromObject(ptr)->size_class];
  } else {
    old_size =
        LargeObjectHeader::GetFromObject(ptr)->size - sizeof(LargeObjectHeader);
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


int posix_memalign(void** ptr, size_t align, size_t size) {
  LOG(kTrace, "posix_memalign: align: %lu, size: %lu", align, size);
  if (UNLIKELY(size == 0)) {                    // Return free-able pointer if
    *ptr = NULL;                                // size == 0.
    return 0;
  }

  const size_t size_needed = align + size;

  if (UNLIKELY(!utils::IsPowerOfTwo(align)) ||  // Power of 2 requirement.
      size_needed < size) {                     // Overflow check.
    return EINVAL;
  }

  uintptr_t start = reinterpret_cast<uintptr_t>(malloc(size_needed));
  if (UNLIKELY(start == 0)) {
    return ENOMEM;
  }

  if (SmallArena.Contains(reinterpret_cast<void*>(start))) {
    start += align - (start % align);
    ScallocAssert((start % align) == 0);
  } else {
    LargeObjectHeader* original_header = LargeObjectHeader::GetFromObject(
        reinterpret_cast<void*>(start));
    start += align - (start % align);
    if ((start % kPageSize) == 0) {
      uintptr_t new_hdr_adr = start - kPageSize;
      if (new_hdr_adr != reinterpret_cast<uintptr_t>(original_header)) {
        reinterpret_cast<LargeObjectHeader*>(new_hdr_adr)->Reset(
            0, original_header);
      }
    }
  }
  *ptr = reinterpret_cast<void*>(start);
  LOG(kTrace, "posix_memalign: returning %p", start);
  return 0;
}


void* memalign(size_t __alignment, size_t __size) {
  void* mem;
  if (posix_memalign(&mem, __alignment, __size)) {
    return NULL;
  }
  return mem;
}


void* aligned_alloc(size_t alignment, size_t size) {
  // The function aligned_alloc() is the same as memalign(), except for the
  // added restriction that size should be a multiple of alignment.
  if (size % alignment != 0) {
    errno = EINVAL;
    return NULL;
  }
  return memalign(alignment, size);
}


void* valloc(size_t __size) {
  return memalign(kPageSize, __size);
}


void* pvalloc(size_t __size) {
  return memalign(kPageSize, utils::PadSize(__size, kPageSize));
}


void malloc_stats(void) {}


int mallopt(int cmd, int value) {
  return 0;
}


bool Ours(const void* p) {
  return SmallArena.Contains(const_cast<void*>(p)) || LargeAllocator::Owns(p);
}

}  // namespace scalloc

#ifndef __THROW
#define __THROW
#endif

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


void* scalloc_aligned_alloc(size_t alignment, size_t size) __THROW {
  return scalloc::aligned_alloc(alignment, size);
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

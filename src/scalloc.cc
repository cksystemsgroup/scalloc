// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "scalloc.h"

#include <errno.h>
#include <string.h>

#include "allocators/large_object_allocator.h"
#include "allocators/dq_sc_allocator.h"
#include "allocators/slab_sc_allocator.h"
#include "common.h"
#include "runtime_vars.h"
#include "scalloc_guard.h"
#include "size_map.h"
#include "thread_cache.h"

static int scallocguard_refcount = 0;
ScallocGuard::ScallocGuard() {
  if (scallocguard_refcount++ == 0) {
    RuntimeVars::InitModule();
    scalloc::SizeMap::InitModule();

    scalloc::DQScAllocator::InitModule();
    scalloc::SlabScAllocator::InitModule();

    free(malloc(1));
    scalloc::ThreadCache::InitPthread();
    free(malloc(1));
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
  if (LIKELY(size <= kMaxSmallSize)) {
    p = ThreadCache::GetCache().Allocate(size);
  } else {
    p = LargeObjectAllocator::Alloc(size);
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
  BlockHeader* hdr = BlockHeader::GetFromObject(p);
  if (hdr->type == kSlab) {
    ThreadCache::GetCache().Free(p, reinterpret_cast<SlabHeader*>(hdr));
  } else if (hdr->type == kLargeObject) {
    LargeObjectAllocator::Free(reinterpret_cast<LargeObjectHeader*>(hdr));
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
  ErrorOut("realloc() not yet implemented.");
}

extern "C" void* scalloc_memalign(size_t __alignment, size_t __size) __THROW {
  ErrorOut("memalign() not yet implemented.");
}

extern "C" int scalloc_posix_memalign(void** ptr, size_t align, size_t size) __THROW {
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

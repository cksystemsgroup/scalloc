// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "glue.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "globals.h"
#include "lab.h"
#include "log.h"
#include "platform/override.h"
#include "size_classes_raw.h"
#include "size_classes.h"
#include "span_pool.h"


namespace scalloc {

cache_aligned const int32_t ClassToObjects[] = {
#define NR_OBJECTS(a, b, c, d) (d),
FOR_ALL_SIZE_CLASSES(NR_OBJECTS)
#undef NR_OBJECTS
};

cache_aligned const int32_t ClassToSize[] = {
#define OBJECT_SIZE(a, b, c, d) (b),
FOR_ALL_SIZE_CLASSES(OBJECT_SIZE)
#undef OBJECT_SIZE
};

cache_aligned const int32_t ClassToSpanSize[] = {
#define SPAN_SIZE(a, b, c, d) (c),
FOR_ALL_SIZE_CLASSES(SPAN_SIZE)
#undef SPAN_SIZE
};

cache_aligned const int32_t ClassToReuseThreshold[] = {
#define REUSE_TH(a, b, c, d) (((d) * kReuseThreshold)/100),
FOR_ALL_SIZE_CLASSES(REUSE_TH)
#undef REUSE_TH
};

// Be careful with order here! Since we define all globals in a single
// translation unit we can rely on order.

cache_aligned Arena core_space;
cache_aligned Arena object_space;
cache_aligned SpanPool span_pool;
cache_aligned ABProvider ab_scheduler;
cache_aligned ScallocGuard StartupExitHook;
/*cache_aligned*/ int32_t ScallocGuardRefcount;
/*cache_aligned*/ int32_t seen_memalign;

#ifdef PROFILE
cache_aligned std::atomic<int32_t> local_frees;
cache_aligned std::atomic<int32_t> remote_frees;
#endif  // PROFILE

void exitHandler() {
#ifdef PROFILE
  span_pool.PrintProfileSummary();
  LOG(kWarning, "free summary: local: %d, remote: %d",
      local_frees.load(), remote_frees.load());
#endif   // PROFILE
}


static void ScallocInit() {
  core_space.Init(kLABSpaceSize, kPageSize, "LAB");
  object_space.Init(kObjectSpaceSize, kObjectSpaceSize, "object");
  span_pool.Init();
  ab_scheduler.Init();

  ab_scheduler.GetMeALAB();
  ReplaceSystemAllocator();
  atexit(exitHandler);
}


scalloc::ScallocGuard::ScallocGuard() {
  if (ScallocGuardRefcount++ == 0) {
    ScallocInit();
  }
}

scalloc::ScallocGuard::~ScallocGuard() {
}

}  // namespace scalloc


#ifndef __THROW
#define __THROW
#endif

extern "C" {
void* scalloc_malloc(size_t size) __THROW {
#ifdef SCALLOC_NO_SAFE_GLOBAL_CONSTRUCTION
  // Since we don't have global initialization dependencies we need to make sure
  // to check whether all components already have been initialized. (e.g. in C++
  // a global variable in a different translation unit can call a runtime
  // function, effectively yielding in an allocation call)
  if (UNLIKELY(scalloc::ScallocGuardRefcount == 0)) {
    scalloc::ScallocGuardRefcount++;
    scalloc::ScallocInit();
  }
#endif  // SCALLOC_NO_SAFE_GLOBAL_CONSTRUCTION
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


void* scalloc_thread_start(void* arg) {
  scalloc::ab_scheduler.GetMeALAB();

  ScallocStartArgs fake_args = *(reinterpret_cast<ScallocStartArgs*>(arg));
  delete reinterpret_cast<ScallocStartArgs*>(arg);
  return fake_args.real_start(fake_args.real_args);
}
}

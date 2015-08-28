// Copyright (c) 2015, the Scal Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_GLUE_H_
#define SCALLOC_GLUE_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "globals.h"
#include "lab.h"
#include "large-objects.h"
#include "log.h"
#include "size_classes.h"
#include "span.h"

namespace scalloc {

extern int seen_memalign;

class ScallocGuard {
 public:
  always_inline ScallocGuard();
  always_inline ~ScallocGuard();
};


always_inline void* malloc(const size_t size) {
  LOG(kTrace, "malloc: size: %lu", size);
  void* obj = ab_scheduler.GetAB().Allocate(size);
  LOG(kTrace, "returning %p", obj);
  // errno is set in a slow path as soon as we know we cannot serve the request.
  // See core.h
  // */
  return obj;
}


always_inline void free(void* p) {
  // No need to check whether p is NULL here since it will fall through the fast
  // path anyways.

  if (LIKELY(object_space.Contains(p))) {
    ab_scheduler.GetAB().Free(p);
  } else {
    // We are in the path for super large objects. Check for NULL here.
    if (UNLIKELY(p == NULL)) {
      return;
    }
    LargeObject::Free(p);
  }
}


always_inline void* calloc(size_t nmemb, size_t size) {
  LOG(kTrace, "calloc: size: %lu", size);
  const size_t malloc_size = nmemb * size;
  if ((size != 0) && (malloc_size / size) != nmemb) {
    return NULL;
  }
  void* result = malloc(malloc_size);  // malloc() sets errno
  if (result != NULL) {
    memset(result, 0, malloc_size);
  }
  return result;
}


always_inline void* realloc(void* ptr, size_t size) {
  LOG(kTrace, "realloc: p: %p, size: %lu", ptr, size);
  if (UNLIKELY(ptr == NULL)) {
    return malloc(size);
  }
  void* new_obj = NULL;
  if (LIKELY(object_space.Contains(ptr))) {
    Span* s = Span::FromObject(ptr);
    const size_t old_size = ClassToSize[s->size_class()];
    if (old_size >= size) {
      return ptr;
    }
    new_obj = malloc(size);
    if (new_obj == nullptr) return nullptr;
    memmove(new_obj, ptr, old_size);
    free(ptr);
  } else {
    const size_t old_size = LargeObject::PayloadSize(ptr);
    if (old_size >= size) {
      return ptr;
    }
    new_obj = malloc(size);
    if (new_obj == nullptr) return nullptr;
    memmove(new_obj, ptr, old_size);
    free(ptr);
  }
  return new_obj;
}


always_inline int posix_memalign(void** ptr, size_t align, size_t size) {
  LOG(kTrace, "posix memalign: size: %lu", size);
  if (seen_memalign == 0) {
    seen_memalign = 1;
  }

  // Return free-able pointer for size 0.
  if (UNLIKELY(size == 0)) {
    *ptr = NULL;
    return 0;
  }

  const size_t size_needed = align + size;

  // TODO: check align for power of 2
  // TODO: check size_needed for overflow

  uintptr_t start = reinterpret_cast<uintptr_t>(malloc(size_needed));
  if (UNLIKELY(start == 0)) {
    return ENOMEM;
  }

  uintptr_t new_start = start + align - (start % align);
  if (new_start != start) {
    // We add a magicnumber to force recalculation of free() address. This is
    // valid because we only transition into a slow path but handle the
    // free still correct.
    *(reinterpret_cast<uint32_t*>(new_start - sizeof(uint32_t))) = 0xAAAAAAAA;
  }
  *ptr = reinterpret_cast<void*>(new_start);
  return 0;
}


always_inline void* memalign(size_t __alignment, size_t __size) {
  void* mem;
  if (posix_memalign(&mem, __alignment, __size)) {
    return NULL;
  }
  return mem;
}


always_inline void* aligned_alloc(size_t alignment, size_t size) {
  // The function aligned_alloc() is the same as memalign(), except for the
  // added restriction that size should be a multiple of alignment.
  if (size % alignment != 0) {
    errno = EINVAL;
    return NULL;
  }
  return memalign(alignment, size);
}


always_inline void* valloc(size_t __size) {
  return memalign(kPageSize, __size);
}


always_inline void* pvalloc(size_t __size) {
  return memalign(kPageSize, PadSize(__size, kPageSize));
}


inline void malloc_stats(void) {
}


inline int mallopt(int cmd, int value) {
  return 0;
}

}  // namespace scalloc

#endif  // SCALLOC_GLUE_H_


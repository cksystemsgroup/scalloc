// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_OVERRIDE_GCC_WEAK_H_
#define SCALLOC_OVERRIDE_GCC_WEAK_H_

#include <stddef.h>  // size_t
#include <sys/cdefs.h>  // __THROW

#include "glue.h"
#include "platform/globals.h"

#ifndef __THROW
#define __THROW
#endif

#define ALIAS(scalloc_fn) __attribute__ ((weak, alias(#scalloc_fn)))

extern "C" {
  void* malloc(size_t size) __THROW                 ALIAS(scalloc_malloc);
  void free(void* p) __THROW                        ALIAS(scalloc_free);
  void cfree(void* p) __THROW                       ALIAS(scalloc_free);
  void* calloc(size_t nmemb, size_t size) __THROW   ALIAS(scalloc_calloc);
  void* realloc(void* ptr, size_t size) __THROW     ALIAS(scalloc_realloc);
  void* memalign(size_t __alignment, size_t __size) __THROW
      ALIAS(scalloc_memalign);
  void* aligned_alloc(size_t alignment, size_t size) __THROW
      ALIAS(scalloc_aligned_alloc);
  int posix_memalign(void** ptr, size_t align, size_t size) __THROW
      ALIAS(scalloc_posix_memalign);
  void* valloc(size_t __size) __THROW               ALIAS(scalloc_valloc);
  void* pvalloc(size_t __size) __THROW              ALIAS(scalloc_pvalloc);
  void malloc_stats(void) __THROW
      ALIAS(scalloc_malloc_stats);
  int mallopt(int cmd, int value) __THROW           ALIAS(scalloc_mallopt);
}

#undef ALIAS

namespace scalloc {

always_inline void ReplaceSystemAllocator() {}

}

#endif  // SCALLOC_OVERRIDE_GCC_WEAK_H_

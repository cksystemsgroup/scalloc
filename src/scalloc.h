// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SCALLOC_H_
#define SCALLOC_SCALLOC_H_

#include <stddef.h>  // size_t
#include <sys/cdefs.h>  // for __THROW

#ifndef __THROW
#define __THROW ""
#endif

#define ALIAS(scalloc_fn) __attribute__ ((alias(#scalloc_fn)))

extern "C" {
  void* malloc(size_t size) __THROW                                   ALIAS(scalloc_malloc);            // NOLINT
  void free(void* p) __THROW                                          ALIAS(scalloc_free);              // NOLINT
  void cfree(void* p) __THROW                                         ALIAS(scalloc_free);              // NOLINT
  void* calloc(size_t nmemb, size_t size) __THROW                     ALIAS(scalloc_calloc);            // NOLINT

  void* realloc(void* ptr, size_t size) __THROW                       ALIAS(scalloc_realloc);           // NOLINT
  void* memalign(size_t __alignment, size_t __size) __THROW           ALIAS(scalloc_memalign);          // NOLINT
  int posix_memalign(void** ptr, size_t align, size_t size) __THROW   ALIAS(scalloc_posix_memalign);    // NOLINT
  void* valloc(size_t __size) __THROW                                 ALIAS(scalloc_valloc);            // NOLINT
  void* pvalloc(size_t __size) __THROW                                ALIAS(scalloc_pvalloc);           // NOLINT
  void malloc_stats(void) __THROW                                     ALIAS(scalloc_malloc_stats);      // NOLINT
  int mallopt(int cmd, int value) __THROW                             ALIAS(scalloc_mallopt);           // NOLINT
}

#endif  // SCALLOC_SCALLOC_H_

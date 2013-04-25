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
  void* malloc(size_t size) __THROW                   ALIAS(scalloc_malloc);
  void free(void* p) __THROW                          ALIAS(scalloc_free);
  void cfree(void* p) __THROW                         ALIAS(scalloc_free);
}

#endif  // SCALLOC_SCALLOC_H_

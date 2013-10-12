// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SCALLOC_H_
#define SCALLOC_SCALLOC_H_

#include <stddef.h>  // size_t

namespace scalloc {
  void* malloc(size_t size);
  void free(void* p);
  void* calloc(size_t nmemb, size_t size);
  void* realloc(void* ptr, size_t size);
  void* memalign(size_t __alignment, size_t __size);
  int posix_memalign(void** ptr, size_t align, size_t size);
  void* valloc(size_t __size);
  void* pvalloc(size_t __size);
  void malloc_stats(void);
  int mallopt(int cmd, int value);

namespace extension {

}
  bool Ours(const void* p);
}  // namespace scalloc

#endif  // SCALLOC_SCALLOC_H_

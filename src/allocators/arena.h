// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_ARENA_H_
#define SCALLOC_ALLOCATORS_ARENA_H_

#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>  // madvise

#include "assert.h"
#include "common.h"

class Arena {
 public:
  void Init(size_t size);
  void* Allocate(const size_t size);
  void Free(void* p, size_t len);
  bool Contains(void* p);

 private:
  uintptr_t start_;
  uintptr_t current_;
  size_t size_;
} cache_aligned;

inline bool Arena::Contains(void* p) {
  return (reinterpret_cast<uintptr_t>(p) ^ start_) < size_;
}

inline void* Arena::Allocate(const size_t size) {
  void* p =  reinterpret_cast<void*>(__sync_fetch_and_add(&current_, size));
  if (reinterpret_cast<uintptr_t>(p) > (start_ + size_)) {
    Fatal("arena: out of memory");
  }
  return p;
}

inline void Arena::Free(void* p, size_t len) {
  if (reinterpret_cast<uintptr_t>(p) > current_) {
    Fatal("arena: invalid free pointer");
  }
  if (madvise(p, len, MADV_DONTNEED) == -1) {
    Fatal("arena: madvise failed. errno: %lu", errno);
  }
}

#endif  // SCALLOC_ALLOCATORS_ARENA_H_

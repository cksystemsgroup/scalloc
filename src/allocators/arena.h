// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_ARENA_H_
#define SCALLOC_ALLOCATORS_ARENA_H_

#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>  // madvise

#include <atomic>

#include "assert.h"
#include "common.h"
#include "log.h"

#define ARENA_LOG "arena"

namespace scalloc {

class Arena {
 public:
  void Init(const size_t size);
  void* Allocate(const size_t size);
  bool Contains(const void* p);

 private:
  std::atomic<uintptr_t> cur_;
  uintptr_t start_;
  size_t size_;
} cache_aligned;


inline bool Arena::Contains(const void* p) {
  return (reinterpret_cast<uintptr_t>(p) ^ start_) < size_;
}


inline void* Arena::Allocate(const size_t size) {
  void* p = reinterpret_cast<void*>(cur_.fetch_add(size));
  if (reinterpret_cast<uintptr_t>(p) > (start_ + size_)) {
    Fatal("[Arena] oom");
  }
  LOG_CAT(ARENA_LOG, kTrace, "allocate: %lu", size);
  return p;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_ARENA_H_

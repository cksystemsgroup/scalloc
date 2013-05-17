// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PROFILER_H_
#define SCALLOC_PROFILER_H_

#include <stddef.h>  // size_t

#include "common.h"

namespace scalloc {

class Profiler {

 public:
  inline void LogAllocation(size_t size) {
    allocation_count_++;
    sizeclass_histogram_[Log2(size)]++;
  }
  inline void LogDeallocation(size_t blocksize = 0, bool remote = false) {
    remote ? remote_free_count_++ : local_free_count_++;
  }

 private:
  uint32_t sizeclass_histogram_[33] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0 // for everythong larger than 1<<32
  };
  uint64_t allocation_count_;
  uint64_t local_free_count_ = 0;
  uint64_t remote_free_count_ = 0;

} cache_aligned;

} //namespace scalloc
#endif  // SCALLOC_PROFILER_H_

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_DQ_SC_ALLOCATOR_H_
#define SCALLOC_ALLOCATORS_DQ_SC_ALLOCATOR_H_

#include <pthread.h>
#include <stdlib.h>

#include "common.h"
#include "distributed_queue.h"
#include "freelist.h"
#include "page_heap.h"
#include "size_map.h"

namespace scalloc {

class DQScAllocator {
 public:
  static void InitModule();
  static DQScAllocator& Instance();

  void Init();
  void Free(void* p, const size_t sc, const size_t dq_id);
  void* Allocate(const size_t sc,
                 const size_t dq_id,
                 const size_t tid,
                 bool* steal_failed);

 private:
  DistributedQueue dqs_[kNumClasses] cache_aligned;
} cache_aligned;

always_inline DQScAllocator& DQScAllocator::Instance() {
  static DQScAllocator singleton;
  return singleton;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_DQ_SC_ALLOCATOR_H_

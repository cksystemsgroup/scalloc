// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_BLOCK_POOL_H_
#define SCALLOC_ALLOCATORS_BLOCK_POOL_H_

#include <pthread.h>
#include <stdlib.h>

#include "assert.h"
#include "common.h"
#include "distributed_queue.h"
#include "headers.h"
#include "size_classes.h"
#include "span_pool.h"
#include "utils.h"

namespace scalloc {

class BlockPool {
 public:
  static void Init();
  static BlockPool& Instance() { return block_pool_; }

  inline BlockPool() {}
  void Free(void* p, const size_t sc, const size_t tid);
  void* Allocate(const size_t sc, const size_t tid, SpanHeader** block);

 private:
  static BlockPool block_pool_ cache_aligned;

  DistributedQueue dqs_[kNumClasses] cache_aligned;

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(BlockPool);
} cache_aligned;


inline void BlockPool::Free(void* p,
                            const size_t sc,
                            const size_t tid) {
  dqs_[sc].EnqueueAt(p, tid % utils::Parallelism());
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_BLOCK_POOL_H_

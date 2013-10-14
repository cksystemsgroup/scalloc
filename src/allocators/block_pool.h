// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_BLOCK_POOL_H_
#define SCALLOC_ALLOCATORS_BLOCK_POOL_H_

#include "block_header.h"
#include "common.h"
#include "distributed_queue.h"
#include "size_map.h"
#include "utils.h"

namespace scalloc {

class BlockPool {
 public:
  static void Init();
  static BlockPool& Instance();

  void Free(void* p, const size_t sc, const size_t dq_id);
  void* Allocate(const size_t sc,
                 const size_t dq_id,
                 const size_t tid,
                 SpanHeader** block);

 private:
  static BlockPool block_pool_ cache_aligned;

  DistributedQueue dqs_[kNumClasses] cache_aligned;
};

inline BlockPool& BlockPool::Instance() {
  return block_pool_;
}

inline void BlockPool::Free(void* p,
                            const size_t sc,
                            const size_t dq_id) {
  dqs_[sc].EnqueueAt(p, dq_id % utils::Cpus());
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_BLOCK_POOL_H_

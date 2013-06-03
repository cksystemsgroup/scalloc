// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_SPAN_POOL_H_
#define SCALLOC_ALLOCATORS_SPAN_POOL_H_

#include <sys/mman.h>

#include "common.h"
#include "distributed_queue.h"

namespace scalloc {

class SpanPool {
 public:
  static void InitModule();
  static SpanPool& Instance();

  void Refill(const size_t refill);
  void* Get();
  void Put(void* p);

 private:
  static SpanPool page_heap_ cache_aligned;
  static const size_t kSpanPoolBackends = 32;
  static size_t __thread refill_ cache_aligned;

  DistributedQueue page_pool_ cache_aligned;

  void* RefillOne();
};

always_inline SpanPool& SpanPool::Instance() {
  return page_heap_;
}

always_inline void SpanPool::Put(void* p) {
  LOG(kTrace, "[SpanPool]: put: %p", p);
  page_pool_.Enqueue(p);
}

always_inline void* SpanPool::Get() {
  LOG(kTrace, "[SpanPool]: get request");
  void* result = page_pool_.Dequeue();
  if (UNLIKELY(result == NULL)) {
    return RefillOne();
  }
  LOG(kTrace, "[SpanPool]: get: %p", result);
  return result;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_SPAN_POOL_H_

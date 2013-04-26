// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOCATORS_PAGE_HEAP_H_
#define SCALLOC_ALLOCATORS_PAGE_HEAP_H_

#include "common.h"
#include "distributed_queue.h"

namespace scalloc {

class PageHeap {
 public:
  static void InitModule();
  static PageHeap* GetHeap();

  void* Get();
  void Put(void* p);
  void AsyncRefill();

 private:
  static const size_t kPageHeapBackends = 4;
  static const size_t kPageRefill = 24;

  static PageHeap page_heap_ cache_aligned;

  DistributedQueue page_pool_ cache_aligned;
};

always_inline PageHeap* PageHeap::GetHeap() {
  return &page_heap_;
}

always_inline void PageHeap::Put(void* p) {
  LOG(kTrace, "[PageHeap]: put: %p", p);
  page_pool_.Enqueue(p);
}

always_inline void* PageHeap::Get() {
 REDO:
  void* result = page_pool_.Dequeue();
  if (UNLIKELY(result == NULL)) {
    AsyncRefill();
    goto REDO;
  }
  LOG(kTrace, "[PageHeap]: get: %p", result);
  return result;
}

}  // namespace scalloc

#endif  // SCALLOC_ALLOCATORS_PAGE_HEAP_H_

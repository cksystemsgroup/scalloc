// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "collector.h"

#include <pthread.h>

#include "span_pool.h"

namespace scalloc {

cache_aligned Collector Collector::collector_;
cache_aligned Stack Collector::work_queue_;
cache_aligned TypedAllocator<Collector::Operation>* Collector::op_allocator_;


void Collector::Init(TypedAllocator<Operation>* op_alloc){
  op_allocator_ = op_alloc;
  work_queue_.Init();
  pthread_t pid;
  pthread_create(&pid, NULL, Collector::Collect, NULL);
}


void Collector::Put(void* p, size_t sc, uint32_t tid) {
  Operation* op = op_allocator_->New();
  op->type = Operation::kPut;
  op->p = p;
  op->sc = sc;
  op->tid = tid;
  work_queue_.Put(PTR(op));
}
  

void* Collector::Collect(void* data) {
  Operation* work = NULL;
  while (true) {
    work = reinterpret_cast<Operation*>(work_queue_.Pop());
    if (work != NULL) {
      
      if (work->type == Operation::kPut) {
        SpanPool::Instance().Put(work->p, work->sc, work->tid);
      }
      
      op_allocator_->Delete(work);
    } else {
      sleep(1);
    }
  }
  return NULL;
}
  
}  // namespace scalloc

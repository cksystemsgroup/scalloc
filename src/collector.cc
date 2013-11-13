// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "collector.h"

#include <pthread.h>

#include "block_header.h"
#include "span_pool.h"

namespace scalloc {

cache_aligned Collector Collector::collector_;
cache_aligned DistributedQueue Collector::work_queue_;


void Collector::Init() {
  work_queue_.Init(80);
  pthread_t pid;
  pthread_create(&pid, NULL, Collector::Collect, NULL);
}


void Collector::Put(void* p) {
  work_queue_.Enqueue(p);
}


void* Collector::Collect(void* data) {
  SpanHeader* work = NULL;
  while (true) {
    work = reinterpret_cast<SpanHeader*>(work_queue_.Dequeue());
    if (work != NULL) {
      SpanPool::Instance().Put(work, work->size_class, work->aowner.owner);
    }
  }
  return NULL;
}

}  // namespace scalloc

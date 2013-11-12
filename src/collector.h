// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_COLLECTOR_H_
#define SCALLOC_COLLECTOR_H_

#include "stack-inl.h"
#include "typed_allocator.h"

namespace scalloc {

class Collector {
 public:
  struct Operation {
    enum Type {
      kPut,
    };
    
    Type type;
    void* p;
    size_t sc;
    uint32_t tid;
  };
  
  static void Init(TypedAllocator<Operation>* op_alloc);
  static void Put(void* p, size_t sc, uint32_t tid);

 private:
  static void* Collect(void* data);

  static TypedAllocator<Operation>* op_allocator_;
  static Collector collector_;
  static Stack work_queue_;
};
  
}  // namespace scalloc

#endif  // SCALLOC_COLLECTOR_H_

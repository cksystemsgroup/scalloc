// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_STACK_INL_H_
#define SCALLOC_STACK_INL_H_

#include <cstddef>  // NULL

namespace scalloc {

class SequentialStack {
 public:
  inline SequentialStack() {
    top_ = NULL;
  }

  inline void Push(void* p) {
    *(reinterpret_cast<void**>(p)) = top_;
    top_ = p;
  }

  inline void* Pop() {
    void* ret = top_;
    if (ret == NULL) {
      return NULL;
    }
    top_ = *(reinterpret_cast<void**>(top_));
    return ret;
  }

 private:
  void* top_;
};

}  // namespace scalloc

#endif  // SCALLOC_STACK_INL_H_

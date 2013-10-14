// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ALLOC_H_
#define SCALLOC_ALLOC_H_

namespace scalloc {

template<typename T>
class TypedAllocator {
 public:
  virtual T* New() = 0;
  virtual void Delete(T* ptr) = 0;
};

}  // namespace scalloc

#endif  // SCALLOC_ALLOC_H_

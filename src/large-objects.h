// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LARGE_OBJECTS_H_
#define SCALLOC_LARGE_OBJECTS_H_

#include <stdint.h>

#include <new>

#include "globals.h"
#include "utils.h"

namespace scalloc {

class LargeObject {
 public:
  static always_inline void* Allocate(size_t size);
  static always_inline void Free(void* p);
  static always_inline size_t ObjectSize(void* p);

 private:
  always_inline explicit LargeObject(size_t size);
  always_inline void* ObjectStart();

  always_inline size_t size() { return actual_size_; }
  size_t actual_size_;
};


void* LargeObject::Allocate(size_t size) {
  const size_t actual_size = size + sizeof(LargeObject);
  LargeObject* obj = new(SystemMmapFail(actual_size)) LargeObject(actual_size);
  return obj->ObjectStart();
}


void LargeObject::Free(void* p) {
  LargeObject* obj = reinterpret_cast<LargeObject*>(
      reinterpret_cast<intptr_t>(p) - sizeof(LargeObject));
  if (munmap(obj, obj->size()) != 0) {
    Fatal("munmap failed");
  }
}


size_t LargeObject::ObjectSize(void* p) {
  LargeObject* obj = reinterpret_cast<LargeObject*>(
      reinterpret_cast<intptr_t>(p) - sizeof(LargeObject));
  return obj->size();
}


LargeObject::LargeObject(size_t size)
    : actual_size_(size) {
}


void* LargeObject::ObjectStart() {
  return reinterpret_cast<void*>(
      reinterpret_cast<intptr_t>(this) + sizeof(*this));
}

}  // namespace scalloc

#endif  // SCALLOC_LARGE_OBJECTS_H_


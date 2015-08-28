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
  static always_inline size_t PayloadSize(void* p);

 private:
  static const uint64_t kMagic = 0xAAAAAAAAAAAAAAAA;

  static always_inline LargeObject* FromMutatorPtr(void* p);

  always_inline explicit LargeObject(size_t size);
  always_inline void* ObjectStart();
  always_inline bool Validate();

  always_inline size_t payload_size() { return actual_size_ - sizeof(*this); }
  always_inline size_t actual_size() { return actual_size_; }

  size_t actual_size_;
  uint64_t magic_;
};


bool LargeObject::Validate() {
  return magic_ == kMagic;
}


LargeObject* LargeObject::FromMutatorPtr(void* p) {
  // LargeObject is always allocated using mmap, hence it is page aligned.
  LargeObject* obj = reinterpret_cast<LargeObject*>(
      reinterpret_cast<intptr_t>(p) & kPageNrMask);
  if (!obj->Validate()) {
    Fatal("invalid large object: %p", p);
  }
  return obj;
}


void* LargeObject::Allocate(size_t size) {
  const size_t actual_size = PadSize(size + sizeof(LargeObject), kPageSize);
  LargeObject* obj = new(SystemMmapFail(actual_size)) LargeObject(actual_size);
#ifdef DEBUG
  // Force the check by going through the mutator pointer.
  obj = LargeObject::FromMutatorPtr(obj->ObjectStart());
#endif  // DEBUG
  return obj->ObjectStart();
}


void LargeObject::Free(void* p) {
  LargeObject* obj = FromMutatorPtr(p);
  if (munmap(obj, obj->actual_size()) != 0) {
    Fatal("munmap failed");
  }
}


size_t LargeObject::PayloadSize(void* p) {
  LargeObject* obj = FromMutatorPtr(p);
  return obj->payload_size();
}


LargeObject::LargeObject(size_t size)
    : actual_size_(size)
    , magic_(kMagic) {
}


void* LargeObject::ObjectStart() {
  return reinterpret_cast<void*>(
      reinterpret_cast<intptr_t>(this) + sizeof(*this));
}

}  // namespace scalloc

#endif  // SCALLOC_LARGE_OBJECTS_H_


// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "scalloc.h"

#include <errno.h>

#include "allocators/large_object_allocator.h"
#include "common.h"

namespace scalloc {

always_inline void* malloc(const size_t size) {
  void* p;
  if (size > kMaxSmallSize) {
    // large allocation
    p = LargeObjectAllocator::Alloc(size);
  } else {
    // TODO(mlippautz): small allocation
  }
  if (UNLIKELY(p == NULL)) {
    errno = ENOMEM;
  }
  return p;
}

always_inline void free(void* p) {
  if (UNLIKELY(p == NULL)) {
    return;
  }
  BlockHeader* hdr = BlockHeader::GetFromObject(p);
  if (hdr->type == kSlab) {
    // TODO(mlippautz): small free
  } else if (hdr->type == kLargeObject) {
    LargeObjectAllocator::Free(reinterpret_cast<LargeObjectHeader*>(hdr));
  }
  return;
}

}  // namespace scalloc

extern "C" void* scalloc_malloc(size_t size) __THROW {
  return scalloc::malloc(size);
}

extern "C" void scalloc_free(void* p) __THROW {
  scalloc::free(p);
}

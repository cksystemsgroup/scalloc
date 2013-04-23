// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BLOCK_HEADER_H_
#define SCALLOC_BLOCK_HEADER_H_

#include "common.h"
#include "freelist.h"
#include "log.h"

enum BlockType {
  kSlab,
  kLargeObject,
  kForward
};

class BlockHeader {
 public:
  static BlockHeader* GetFromObject(void* p);

  BlockType type;
};

class ForwardHeader : public BlockHeader {
 public:
  BlockHeader* forward;

  inline void Reset(BlockHeader* forward) {
    this->type = kForward;
    this->forward = forward;
  }
};

class SlabHeader : public BlockHeader {
 public:
  // read-only properties

  size_t size_class;
  size_t remote_flist;

  // mostly read properties

  bool active;
  size_t owner;

  // thread-local read/write properties

  uint64_t in_use;
  Freelist flist;

  inline void Reset() {
    this->type = kSlab;
  }
} cache_aligned;

class LargeObjectHeader : public BlockHeader {
 public:
  size_t size;

  inline void Reset(size_t size) {
    this->type = kLargeObject;
    this->size = size;
  }
} cache_aligned;

always_inline BlockHeader* BlockHeader::GetFromObject(void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
  if (UNLIKELY(ptr % kSystemPageSize == 0)) {
    BlockHeader* bh = reinterpret_cast<BlockHeader*>(ptr - kSystemPageSize);
    if (bh->type == kForward) {
      bh = reinterpret_cast<ForwardHeader*>(bh)->forward;
    }
  }
  uintptr_t page_ptr = ptr & ~(kSystemPageSize - 1);
  BlockHeader* bh = reinterpret_cast<BlockHeader*>(page_ptr);
  switch (bh->type) {
  case kForward:
    return reinterpret_cast<ForwardHeader*>(bh)->forward;
  case kSlab:
  case kLargeObject:
    return bh;
  default:
    ErrorOut("unknown block header. type: %d, ptr: %p, page_ptr: %p",
             bh->type, p, reinterpret_cast<void*>(page_ptr));
  }
  // unreachable...
  return NULL;
}

#endif  // SCALLOC_BLOCK_HEADER_H_

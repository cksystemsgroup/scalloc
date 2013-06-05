// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BLOCK_HEADER_H_
#define SCALLOC_BLOCK_HEADER_H_

#include "common.h"
#include "freelist.h"
#include "log.h"
#include "runtime_vars.h"
#include "size_map.h"

enum BlockType {
  UNDEF,
  kSlab,
  kLargeObject,
  kForward
};

class Header {
 public:
  static Header* GetFromObject(void* p);

  BlockType type;
};
typedef Header BlockHeader;

class PageHeader : public Header {
 public:
  Header* forward;

  inline void Reset(Header* forward) {
    this->type = kForward;
    this->forward = forward;
  }
};
typedef PageHeader ForwardHeader;

struct ActiveOwner {
  union {
    struct {
      bool active : 1;
      uint64_t owner : 63;
    };
    uint64_t raw;
  };

  inline ActiveOwner() {}

  inline ActiveOwner(const bool active, const uint64_t owner) {
    Reset(active, owner);
  }

  inline void Reset(const bool active, const uint64_t owner) {
    this->active = active;
    this->owner = owner;
  }
};

class SpanHeader : public Header {
 public:
  static SpanHeader* GetFromObject(void* p) {
    //const size_t span_size = RuntimeVars::SystemPageSize() * kPageMultiple;
    const size_t span_size = 1UL << 28;
    return reinterpret_cast<SpanHeader*>(reinterpret_cast<uintptr_t>(p) & ~(span_size - 1));
  }

  // read-only properties

  struct {
  size_t size_class;
  size_t remote_flist;
  } cache_aligned;

  // mostly read properties

  cache_aligned ActiveOwner aowner;

  // thread-local read/write properties

  struct {
  uint64_t in_use;
  Freelist flist;
  } cache_aligned;

  inline void Reset(const size_t size_class, const size_t remote_flist) {
    this->type = kSlab;
    this->size_class = size_class;
    this->remote_flist = remote_flist;
    this->in_use = 0;
  }

  // The utilization of the span in percent.
  inline size_t Utilization() {
    return 100 - ((this->flist.Size() * 100) /
        scalloc::SizeMap::Instance().MaxObjectsPerClass(this->size_class));
  }
} cache_aligned;
typedef SpanHeader SlabHeader;

class LargeObjectHeader : public Header {
 public:
  size_t size;

  inline void Reset(size_t size) {
    this->type = kLargeObject;
    this->size = size;
  }
} cache_aligned;

always_inline Header* Header::GetFromObject(void* p) {
  const size_t sys_page_size = RuntimeVars::SystemPageSize();
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
  if (UNLIKELY(ptr % sys_page_size == 0)) {
    Header* bh = reinterpret_cast<Header*>(ptr - sys_page_size);
    if (bh->type == kForward) {
      bh = reinterpret_cast<ForwardHeader*>(bh)->forward;
    }
  }
  uintptr_t page_ptr = ptr & ~(sys_page_size - 1);
  Header* bh = reinterpret_cast<Header*>(page_ptr);
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

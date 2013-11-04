// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_BLOCK_HEADER_H_
#define SCALLOC_BLOCK_HEADER_H_

#include "assert.h"
#include "common.h"
#include "freelist-inl.h"
#include "log.h"
#include "size_classes.h"

enum BlockType {
  kUndef,
  kSlab,
  kLargeObject,
  kNumHeaders
};


struct ActiveOwner {
  inline ActiveOwner() {}

  inline ActiveOwner(const bool active, const uint64_t owner) {
    Reset(active, owner);
  }

  inline void Reset(const bool active, const uint64_t owner) {
    this->active = active;
    this->owner = owner;
  }

  union {
    struct {
      bool active : 1;
      uint64_t owner : 63;
    };
    uint64_t raw;
  };
};


class Header {
 public:
  BlockType type;

  DISALLOW_ALLOCATION();
};


class SpanHeader : public Header {
 public:
  static inline SpanHeader* GetFromObject(void* p) {
    return reinterpret_cast<SpanHeader*>
        (reinterpret_cast<uintptr_t>(p) & kVirtualSpanMask);
  }

  inline void Init(const size_t size_class, const size_t id) {
    this->type = kSlab;
    this->size_class = size_class;
    this->remote_flist = id;
    this->flist_aligned_blocksize_offset =
        (reinterpret_cast<uintptr_t>(this) + sizeof(SpanHeader))
            % scalloc::ClassToSize[size_class];
    this->aowner.owner = id;
    this->aowner.active = true;
  }

  // The utilization of the span in percent.
  inline size_t Utilization() {
    return flist.Utilization();
  }

  // read-only properties
  size_t size_class;
  size_t remote_flist;
  size_t flist_aligned_blocksize_offset;
#define READ_SZ                               \
  (sizeof(size_class) +                       \
  sizeof(remote_flist) +                      \
  sizeof(flist_aligned_blocksize_offset))
  char pad1[CACHELINE_SIZE - (READ_SZ % CACHELINE_SIZE)];  // NOLINT
#undef READ_SZ

  // mostly read properties
  cache_aligned ActiveOwner aowner;

  // thread-local read/write properties
  Freelist flist;
#define RW_SZ (sizeof(flist))
  char pad2[CACHELINE_SIZE - (RW_SZ % CACHELINE_SIZE)];  // NOLINT
#undef RW_SZ
} cache_aligned;


class LargeObjectHeader : public Header {
 public:
  static inline LargeObjectHeader* GetFromObject(void* p) {
    uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
    uintptr_t page_ptr = ptr & ~(kPageSize - 1);

    if (UNLIKELY(ptr == page_ptr)) {
      // We have a large object that is page aligned. This means that it was
      // aligned and that we can find a valid header in the page before.
      page_ptr -= kPageSize;
      if (reinterpret_cast<LargeObjectHeader*>(page_ptr)->fwd != NULL) {
        // Because of alignment, we may actually have to follow a forward
        // pointer to get to the actual header.
        page_ptr = reinterpret_cast<uintptr_t>(
            reinterpret_cast<LargeObjectHeader*>(page_ptr)->fwd);
      }
    }

#ifdef DEBUG
    Header* bh = reinterpret_cast<Header*>(page_ptr);
    if (UNLIKELY(bh->type != kLargeObject)) {
      Fatal("Invalid large object header. "
            "type: %d, ptr: %p, page ptr: %p\n",
            bh->type, p, reinterpret_cast<void*>(page_ptr));
    }
#endif
    return reinterpret_cast<LargeObjectHeader*>(page_ptr);
  }

  inline void Reset(size_t size, LargeObjectHeader* fwd = NULL) {
    this->type = kLargeObject;
    this->size = size;
    this->fwd = fwd;
  }

  size_t size;
  LargeObjectHeader* fwd;
} cache_aligned;

#endif  // SCALLOC_BLOCK_HEADER_H_

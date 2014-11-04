// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
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

  always_inline ActiveOwner(const bool active, const uint64_t owner) {
    Reset(active, owner);
  }

  always_inline void Reset(const bool active, const uint64_t owner) {
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
  void* next;
  void* prev;
  BlockType type;
  uint32_t __pad0;

  DISALLOW_ALLOCATION();
};


#define FIELD_OFFSET(field) \
  (reinterpret_cast<uintptr_t>(&field) - reinterpret_cast<uintptr_t>(this))

#define FIELD_OFFSET_AND_SIZE(field) \
  FIELD_OFFSET(field), sizeof(field)


class SpanHeader : public Header {
 public:
  static always_inline SpanHeader* GetFromObject(const void* p) {
    return reinterpret_cast<SpanHeader*>
        (reinterpret_cast<uintptr_t>(p) & kVirtualSpanMask);
  }

  static inline void PrintHeaderLayout() {
    void* a;
    SpanHeader* dummy = reinterpret_cast<SpanHeader*>(&a);
    dummy->PrintLayout();
  }

  static inline void CheckFieldAlignments() {
    void* a;
    SpanHeader* dummy = reinterpret_cast<SpanHeader*>(&a);
    dummy->CheckAlignments();
  }

  inline void Init(
      const size_t sc, const size_t owner_id, bool reuse_freelist) {
    this->type = kSlab;
    this->size_class = sc;
    this->remote_flist = owner_id;
    this->flist_aligned_blocksize_offset =
        (reinterpret_cast<uintptr_t>(this) + sizeof(SpanHeader))
            % scalloc::ClassToSize[sc];
    this->madvised = 0;
    this->aowner.owner = owner_id;
    this->aowner.active = true;
    if (!reuse_freelist) {
      this->flist.Init(
          PTR(reinterpret_cast<uintptr_t>(this) + sizeof(SpanHeader)),
          sc);
    }
    this->next = NULL;
    this->prev = NULL;
  }

  // The utilization of the span in percent.
  always_inline size_t Utilization() { return flist.Utilization(); }

  // read-only properties
  size_t size_class;
  size_t remote_flist;
  size_t flist_aligned_blocksize_offset;
  size_t madvised;
#define READ_SZ                            \
  (sizeof(next) +                          \
  sizeof(prev) +                           \
  sizeof(type) +                           \
  sizeof(__pad0) +                         \
  sizeof(size_class) +                     \
  sizeof(remote_flist) +                   \
  sizeof(flist_aligned_blocksize_offset)) +\
  sizeof(madvised)
  char __pad1[CACHELINE_SIZE - (READ_SZ % CACHELINE_SIZE)];  // NOLINT
#undef READ_SZ

  // r/w sync properties
  ActiveOwner aowner;
#define SYNC_SZ (sizeof(aowner))
  char __pad2[CACHELINE_SIZE - (SYNC_SZ % CACHELINE_SIZE)];  // NOLINT
#undef SYNC_SZ

  // thread-local read/write properties
#ifdef INCREMENTAL_FREELIST
  scalloc::IncrementalFreelist flist;
#else
  scalloc::BatchedFreelist flist;
#endif  // INCREMENTAL_FREELIST
#define RW_SZ (sizeof(flist))
  char __pad3[CACHELINE_SIZE - (RW_SZ % CACHELINE_SIZE)];  // NOLINT
#undef RW_SZ

 private:
  inline void CheckAlignments() {
    uintptr_t base = reinterpret_cast<uintptr_t>(this);
    if (((reinterpret_cast<uintptr_t>(&aowner) - base) % CACHELINE_SIZE != 0) ||
        ((reinterpret_cast<uintptr_t>(&flist) - base) % CACHELINE_SIZE != 0) ||
        ((reinterpret_cast<uintptr_t>(&__pad3) - base + sizeof(__pad3))
            % CACHELINE_SIZE != 0)) {
      PrintLayout();
      Fatal("span header misaligned");
    }
  }

  inline void PrintLayout() {
    printf("Span header:\n\n"
           "OFFSET\tSIZE\tFIELD\n"
           "%lu\t%lu\tnext\n"
           "%lu\t%lu\tprev\n"
           "%lu\t%lu\ttype\n"
           "%lu\t%lu\t__pad0\n"
           "--- read only (span header) ---\n"
           "%lu\t%lu\tsize_class\n"
           "%lu\t%lu\tremote_flist\n"
           "%lu\t%lu\tflist_aligned_blocksize_offset\n"
           "%lu\t%lu\tmadvised\n"
           "%lu\t%lu\t__pad1\n"
           "--- read/write sync ---\n"
           "%lu\t%lu\taowner\n"
           "%lu\t%lu\t__pad2\n"
           "--- threadlocal write ---\n"
           "%lu\t%lu\tflist\n"
           "%lu\t%lu\t__pad3\n"
           "--- payload @ %lu ---\n\n",
           FIELD_OFFSET_AND_SIZE(next),
           FIELD_OFFSET_AND_SIZE(prev),
           FIELD_OFFSET_AND_SIZE(type),
           FIELD_OFFSET_AND_SIZE(__pad0),
           FIELD_OFFSET_AND_SIZE(size_class),
           FIELD_OFFSET_AND_SIZE(remote_flist),
           FIELD_OFFSET_AND_SIZE(flist_aligned_blocksize_offset),
           FIELD_OFFSET_AND_SIZE(madvised),
           FIELD_OFFSET_AND_SIZE(__pad1),
           FIELD_OFFSET_AND_SIZE(aowner),
           FIELD_OFFSET_AND_SIZE(__pad2),
           FIELD_OFFSET_AND_SIZE(flist),
           FIELD_OFFSET_AND_SIZE(__pad3),
           sizeof(SpanHeader));
  }

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(SpanHeader);
};


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

  static inline void PrintHeaderLayout() {
    LargeObjectHeader dummy;
    dummy.PrintLayout();
  }

  static inline void CheckFieldAlignments() {
    LargeObjectHeader dummy;
    dummy.CheckAlignments();
  }

  inline void Reset(size_t size, LargeObjectHeader* fwd = NULL) {
    this->type = kLargeObject;
    this->size = size;
    this->fwd = fwd;
  }

  size_t size;
  LargeObjectHeader* fwd;
#define READ_SZ                              \
    (sizeof(next) +                          \
     sizeof(prev) +                          \
     sizeof(type) +                          \
     sizeof(__pad0) +                        \
     sizeof(size) +                          \
     sizeof(fwd))
  char __pad1[CACHELINE_SIZE - (READ_SZ % CACHELINE_SIZE)];  // NOLINT
#undef READ_SZ

 private:
  inline void CheckAlignments() {
    if ((reinterpret_cast<uintptr_t>(&__pad1) -
         reinterpret_cast<uintptr_t>(this) +
         sizeof(__pad1)) % CACHELINE_SIZE != 0) {
      PrintLayout();
      Fatal("large object header misaligned");
    }
  }

  inline void PrintLayout() {
    printf("Large object header:\n\n"
           "OFFSET\tSIZE\tFIELD\n"
           "--- read only (base header) ---\n"
           "%lu\t%lu\tnext\n"
           "%lu\t%lu\tprev\n"
           "%lu\t%lu\ttype\n"
           "%lu\t%lu\t__pad0\n"
           "--- read only (span header) ---\n"
           "%lu\t%lu\tsize\n"
           "%lu\t%lu\tfwd\n"
           "%lu\t%lu\t__pad1\n"
           "--- payload @ %lu ---\n\n",
           FIELD_OFFSET_AND_SIZE(next),
           FIELD_OFFSET_AND_SIZE(prev),
           FIELD_OFFSET_AND_SIZE(type),
           FIELD_OFFSET_AND_SIZE(__pad0),
           FIELD_OFFSET_AND_SIZE(size),
           FIELD_OFFSET_AND_SIZE(fwd),
           FIELD_OFFSET_AND_SIZE(__pad1),
           FIELD_OFFSET(__pad1) + sizeof(__pad1));
  }
};


#undef FIELD_OFFSET
#undef FIELD_OFFSET_AND_SIZE

#endif  // SCALLOC_BLOCK_HEADER_H_

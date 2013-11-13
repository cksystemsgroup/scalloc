// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
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
  void* __RESERVED_FOR_LINK_POINTERS;
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
  static inline SpanHeader* GetFromObject(void* p) {
    return reinterpret_cast<SpanHeader*>
        (reinterpret_cast<uintptr_t>(p) & kVirtualSpanMask);
  }

  static inline void PrintHeaderLayout() {
    SpanHeader dummy;
    dummy.PrintLayout();
  }

  static inline void CheckFieldAlignments() {
    SpanHeader dummy;
    dummy.CheckAlignments();
  }

  inline void Init(const size_t size_class, const size_t id, bool reusable) {
    this->type = kSlab;
    this->size_class = size_class;
    this->remote_flist = id;
    this->flist_aligned_blocksize_offset =
        (reinterpret_cast<uintptr_t>(this) + sizeof(SpanHeader))
            % scalloc::ClassToSize[size_class];
    this->aowner.owner = id;
    this->aowner.active = true;
    if (!reusable) {
      this->flist.Init(
          PTR(reinterpret_cast<uintptr_t>(this) + sizeof(SpanHeader)),
          size_class);
    }
  }

  // The utilization of the span in percent.
  inline size_t Utilization() {
    return flist.Utilization();
  }

  // read-only properties
  size_t size_class;
  size_t remote_flist;
  size_t flist_aligned_blocksize_offset;
#define READ_SZ                            \
  (sizeof(__RESERVED_FOR_LINK_POINTERS) +  \
  sizeof(type) +                           \
  sizeof(__pad0) +                         \
  sizeof(size_class) +                     \
  sizeof(remote_flist) +                   \
  sizeof(flist_aligned_blocksize_offset))
  char __pad1[CACHELINE_SIZE - (READ_SZ % CACHELINE_SIZE)];  // NOLINT
#undef READ_SZ

  // r/w sync properties
  ActiveOwner aowner;
#define SYNC_SZ (sizeof(aowner))
  char __pad2[CACHELINE_SIZE - (SYNC_SZ % CACHELINE_SIZE)];  // NOLINT
#undef SYNC_SZ

  // thread-local read/write properties
  scalloc::Freelist flist;
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
           "%lu\t%lu\t__RESERVED_FOR_LINK_POINTERS\n"
           "%lu\t%lu\ttype\n"
           "%lu\t%lu\t__pad0\n"
           "--- read only (span header) ---\n"
           "%lu\t%lu\tsize_class\n"
           "%lu\t%lu\tremote_flist\n"
           "%lu\t%lu\tflist_aligned_blocksize_offset\n"
           "%lu\t%lu\t__pad1\n"
           "--- read/write sync ---\n"
           "%lu\t%lu\taowner\n"
           "%lu\t%lu\t__pad2\n"
           "--- threadlocal write ---\n"
           "%lu\t%lu\tflist\n"
           "%lu\t%lu\t__pad3\n"
           "--- payload @ %lu ---\n\n",
           FIELD_OFFSET_AND_SIZE(__RESERVED_FOR_LINK_POINTERS),
           FIELD_OFFSET_AND_SIZE(type),
           FIELD_OFFSET_AND_SIZE(__pad0),
           FIELD_OFFSET_AND_SIZE(size_class),
           FIELD_OFFSET_AND_SIZE(remote_flist),
           FIELD_OFFSET_AND_SIZE(flist_aligned_blocksize_offset),
           FIELD_OFFSET_AND_SIZE(__pad1),
           FIELD_OFFSET_AND_SIZE(aowner),
           FIELD_OFFSET_AND_SIZE(__pad2),
           FIELD_OFFSET_AND_SIZE(flist),
           FIELD_OFFSET_AND_SIZE(__pad3),
           FIELD_OFFSET(__pad3) + sizeof(__pad3)
           );
  }
} __attribute__((packed));


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
#define READ_SZ                               \
  (sizeof(__RESERVED_FOR_LINK_POINTERS) +     \
   sizeof(type) +                             \
   sizeof(__pad0) +                           \
   sizeof(size) +                             \
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
           "%lu\t%lu\t__RESERVED_FOR_LINK_POINTERS\n"
           "%lu\t%lu\ttype\n"
           "%lu\t%lu\t__pad0\n"
           "--- read only (span header) ---\n"
           "%lu\t%lu\tsize\n"
           "%lu\t%lu\tfwd\n"
           "%lu\t%lu\t__pad1\n"
           "--- payload @ %lu ---\n\n",
           FIELD_OFFSET_AND_SIZE(__RESERVED_FOR_LINK_POINTERS),
           FIELD_OFFSET_AND_SIZE(type),
           FIELD_OFFSET_AND_SIZE(__pad0),
           FIELD_OFFSET_AND_SIZE(size),
           FIELD_OFFSET_AND_SIZE(fwd),
           FIELD_OFFSET_AND_SIZE(__pad1),
           FIELD_OFFSET(__pad1) + sizeof(__pad1)
           );
  }
};


#undef FIELD_OFFSET
#undef FIELD_OFFSET_AND_SIZE

#endif  // SCALLOC_BLOCK_HEADER_H_

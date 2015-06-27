// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ARENA_H_
#define SCALLOC_ARENA_H_

#include <atomic>
#include <cstdint>

#include "globals.h"
#include "platform/assert.h"
#include "utils.h"

namespace scalloc {

class Arena {
 public:
  // Globally constructed, hence we use staged construction.
  always_inline Arena() {}
  always_inline ~Arena() {}

  always_inline void Init(size_t size, size_t alignment, const char* name);
  always_inline bool Contains(const void* p);
  always_inline void* Allocate(size_t size);
  always_inline void* AllocateVirtualSpan();

 private:
  const char* name_;

  // All of the following members hold pointers to their respective locations.
  // Use unsigned to make comparison against size_t easy.
  uintptr_t start_;
  uintptr_t end_;
  uintptr_t len_;

  UNUSED uint8_t pad1_[64 -
      ((sizeof(name_) +
        sizeof(start_) +
        sizeof(end_) +
        sizeof(len_)) % 64)];

  std::atomic<uintptr_t> current_;
  UNUSED uint8_t pad2_[64  -
      ((sizeof(current_)) % 64)];
};


void Arena::Init(size_t size, size_t alignment, const char* name) {
  name_ = name;
  len_ = size;
  bool needs_aligning = false;
  start_ = reinterpret_cast<uintptr_t>(
      SystemMmapGuided(reinterpret_cast<void*>(alignment), size));
  if (start_ == 0) {
    start_ = reinterpret_cast<uintptr_t>(SystemMmap(size + alignment));
    needs_aligning = true;
  }
  if (start_ == 0) {
    Fatal("initial mmap of arena failed. "
          "consult online docs for requirements (overcommit_memory)");
  }
  if (needs_aligning) {
    start_ += alignment - (start_ % alignment);
  }
  ScallocAssert((start_ % alignment) == 0);
  end_ = start_ + size;
  current_.store(start_);
#if defined(SCALLOC_STRICT_DUMP) && defined(MADV_DONTDUMP)
  madvise(reinterpret_cast<void*>(start_), len_, MADV_DONTDUMP);
#endif  // DEBUG && MADV_DONTDUMP

  LOG(kTrace, "arena at %p", this);
}


bool Arena::Contains(const void* p) {
  // Requires proper alignment of start_!
  return LIKELY((reinterpret_cast<uintptr_t>(p) ^ start_) < len_);
}


void* Arena::Allocate(size_t size) {
  LOG(kTrace, "allocate: %lu", size);
  uintptr_t obj = current_.fetch_add(size);
  if (UNLIKELY((obj + size) >= end_)) {
    Fatal("%s arena OOM; start: %p, end: %p, curr: %p",
        name_, start_, end_, current_.load());
  }
  LOG(kTrace, "%s: obj: %p", name_, obj);
#if defined(SCALLOC_STRICT_DUMP) && defined(MADV_DODUMP)
  madvise(reinterpret_cast<void*>(obj & kPageNrMask),
          ((size / kPageSize) + 1) * kPageSize,
          MADV_DODUMP);
#endif  // DEBUG && MADV_DODUMP
  return reinterpret_cast<void*>(obj);
}


void* Arena::AllocateVirtualSpan() {
  LOG(kTrace, "allocate: %lu", kVirtualSpanSize);
  uintptr_t obj = current_.fetch_add(kVirtualSpanSize);
  if (UNLIKELY((obj + kVirtualSpanSize) >= end_)) {
    Fatal("%s arena OOM; start: %p, end: %p, curr: %p",
        name_, start_, end_, current_.load());
  }
  LOG(kTrace, "%s: obj: %p", name_, obj);
#if defined(SCALLOC_STRICT_DUMP) && defined(MADV_DODUMP)
  madvise(reinterpret_cast<void*>(obj), kVirtualSpanSize, MADV_DODUMP);
#endif  // DEBUG && MADV_DODUMP
  return reinterpret_cast<void*>(obj);
}

}  // namespace scalloc

#endif  // SCALLOC_ARENA_H_


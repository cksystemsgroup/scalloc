// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SPAN_H_
#define SCALLOC_SPAN_H_

#include <pthread.h>

#include <new>

#include "core_id.h"
#include "deque.h"
#include "free_list.h"
#include "globals.h"
#include "lock.h"
#include "log.h"
#include "platform/assert.h"
#include "span_pool.h"
#include "stack.h"

namespace scalloc {

#ifdef PROFILE
extern std::atomic<int32_t> local_frees;
extern std::atomic<int32_t> remote_frees;
#endif  // PROFILE


class Span {
 public:
  static const int_fast32_t kAlignTag = 0xAAAAAAAA;

  static always_inline bool IsFloatingOrReusable(int32_t epoch) {
    return !IsFull(epoch) && !IsHot(epoch);
  }

  static always_inline bool IsReusable(int32_t epoch) {
    return (epoch & kEpochReuse) != 0;
  }

  static always_inline bool IsHot(int32_t epoch) {
    return (epoch & kEpochHot) != 0;
  }

  static always_inline bool IsFull(int32_t epoch) {
    return (epoch & kEpochFull);
  }

  static always_inline Span* FromObject(const void* p);
  static always_inline Span* FromSpanLink(DoubleListNode* link);
  static always_inline Span* New(size_t size_class, core_id owner);
  static always_inline void Delete(Span* s);

  always_inline void* Allocate();
  always_inline int32_t Free(void* p, core_id caller);
  always_inline void* AlignToBlockStart(void* p);
  always_inline void MoveRemoteToLocalObjects();

  always_inline size_t size_class();
  always_inline core_id owner();
  always_inline DoubleListNode* SpanLink();
  always_inline int32_t epoch();
  always_inline bool NewMarkHot(int32_t old_epoch);
  always_inline bool NewMarkFull(int32_t old_epoch);
  always_inline bool NewMarkReuse(int32_t old_epoch);
  always_inline void NewMarkFloating();
  always_inline bool TryMarkFloating(int32_t old_epoch);
  always_inline bool TryReviveNew(core_id old_owner, core_id caller);

  always_inline int_fast32_t NrFreeObjects() {
    return NrLocalObjects() + NrRemoteObjects();
  }


 private:
  typedef Stack<64> RemoteFreeList;

  enum EpochBit {
    kHotBit = 31,
    kFullBit = 30,
    kReuseBit = 29,
    kLastValueBit = 28
  };
  enum EpochValue {
    kEpochHot = (1 << kHotBit),
    kEpochFull = (1 << kFullBit),
    kEpochReuse = (1 << kReuseBit),
  };
  enum EpochMask {
    kEpochOnlyValuesMask = (1 << kLastValueBit) - 1,
    kEpochHotMask = kEpochOnlyValuesMask | kEpochHot,
    kEpochFullMask = kEpochOnlyValuesMask | kEpochFull,
    kEpochReuseMask = kEpochOnlyValuesMask | kEpochReuse,
  };

  always_inline int_fast32_t NrRemoteObjects() {
    if (remote_free_list_.Empty()) return 0;
    return remote_free_list_.Length();
  }
  always_inline int_fast32_t NrLocalObjects() {
    return local_free_list_.Length();
  }

  always_inline Span(size_t sc, core_id owner);
  always_inline void CheckAlignments();
  always_inline intptr_t HeaderEnd();

  // This list is used to link up reusable spans in the corresponding core. The
  // first word is also used in the span pool to link up spans.
  DoubleListNode span_link_;

  AtomicCoreID owner_;

  // Epoch counter that even survives traversing of a span through the span
  // pool.
  std::atomic<int32_t> epoch_;

  int32_t size_class_;
  UNUSED  char padding_[8];
  IncrementalFreeList local_free_list_;

  RemoteFreeList remote_free_list_;
};


#define FOR_ALL_SPAN_FIELDS(V)                                                 \
  V(span_link_)                                                                \
  V(owner_)                                                                    \
  V(epoch_)                                                                    \
  V(size_class_)                                                               \
  V(local_free_list_)                                                          \
  V(remote_free_list_)                                                         \


Span* Span::FromObject(const void* p) {
  return reinterpret_cast<Span*>(
      reinterpret_cast<uintptr_t>(p) & kVirtualSpanMask);
}


Span* Span::FromSpanLink(DoubleListNode* link) {
  return reinterpret_cast<Span*>(link);
}


Span* Span::New(size_t size_class, core_id owner) {
  return new(span_pool.Allocate(size_class, owner.tag()))
         Span(size_class, owner);
}


void Span::Delete(Span* s) {
  ScallocAssert(s->span_link_.next() == nullptr);
  ScallocAssert(s->span_link_.prev() == nullptr);
  span_pool.Free(s->size_class(), s, s->owner().tag());
}


void* Span::AlignToBlockStart(void* p) {
  // Check if realigning is needed.
  if (*reinterpret_cast<int_fast32_t*>(
          reinterpret_cast<uintptr_t>(p) - sizeof(int_fast32_t)) == kAlignTag) {
    const uintptr_t d =
        (reinterpret_cast<uintptr_t>(p) - HeaderEnd())
          % ClassToSize[size_class_];
    LOG(kWarning, "found aligned adr: %p", p);
    p = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) - d);
    LOG(kWarning, "  fix to: %p", p);
  }
  return p;
}


Span::Span(size_t size_class, core_id owner)
    : span_link_()
    , owner_(owner)
    , size_class_(size_class)
    , local_free_list_(HeaderEnd(), size_class)
    , remote_free_list_() {
  ScallocAssert(local_free_list_.Length() == ClassToObjects[size_class]);
  ScallocAssert(remote_free_list_.Length() == 0);
  ScallocAssert(owner.value() != nullptr);

  // Mark span as hot.
  NewMarkHot(epoch());

#ifdef DEBUG
  CheckAlignments();
#endif  // DEBUG
}


size_t Span::size_class() { return size_class_; }
core_id Span::owner() { return owner_.load(); }
int32_t Span::epoch() { return epoch_.load(); }


bool Span::TryReviveNew(core_id old_owner, core_id caller) {
  ScallocAssert(caller.value() != nullptr);
  bool ok = owner_.swap(old_owner, caller);
  return ok;
}


intptr_t Span::HeaderEnd() {
  return reinterpret_cast<intptr_t>(this) + sizeof(*this);
}


DoubleListNode* Span::SpanLink() {
  ScallocAssert(&span_link_ == reinterpret_cast<DoubleListNode*>(this));
  return &span_link_;
}


bool Span::NewMarkHot(int32_t old_epoch) {
  old_epoch &= kEpochReuseMask;
  // We want to set only the hot bit and a new value.
  int32_t new_epoch = ((old_epoch + 1) | kEpochHot) & kEpochHotMask;
  return epoch_.compare_exchange_strong(old_epoch, new_epoch);
}


bool Span::NewMarkFull(int32_t old_epoch) {
  // Reuse bit can be set, but nothing else.
  old_epoch &= kEpochReuseMask;
  int32_t new_epoch = ((old_epoch + 1) | kEpochFull) & kEpochFullMask;
  return epoch_.compare_exchange_strong(old_epoch, new_epoch);
}


bool Span::NewMarkReuse(int32_t old_epoch) {
  // None of the bits should be set.
  old_epoch &= kEpochOnlyValuesMask;
  int32_t new_epoch = ((old_epoch  + 1) | kEpochReuse) & kEpochReuseMask;
  return epoch_.compare_exchange_strong(old_epoch, new_epoch);
}


void Span::NewMarkFloating() {
  // There's no race in this one as we always go through the hot state which
  // is already exclusive.
  epoch_.store(epoch_.load() & kEpochOnlyValuesMask);
}


bool Span::TryMarkFloating(int32_t old_epoch) {
  int32_t new_epoch = old_epoch & kEpochOnlyValuesMask;
  return epoch_.compare_exchange_strong(old_epoch, new_epoch);
}


void Span::CheckAlignments() {
  // Dynamically check field alignment for 32bit.
#define CHECK_FIELD(field)                                                     \
  if ((reinterpret_cast<intptr_t>(&field) & 0x3) != 0) {                       \
    Fatal("Field " #field " not properly aligned.");                           \
  }

FOR_ALL_SPAN_FIELDS(CHECK_FIELD)
#undef CHECK_FIELD

#define CHECK_CACHE_ALIGNED_ADR(ID) do {                                       \
  if ((reinterpret_cast<intptr_t>(ID) % kCacheLineSize) != 0) {                \
    Fatal("Address " #ID " not %d byte aligned: %p", kCacheLineSize, ID);      \
  }                                                                            \
} while (0)

CHECK_CACHE_ALIGNED_ADR(&remote_free_list_);
CHECK_CACHE_ALIGNED_ADR(HeaderEnd());
#undef CHECK_CACHE_ALIGNED_ADR
}


void* Span::Allocate() {
  return local_free_list_.Pop();
}


int32_t Span::Free(void* p, core_id caller) {
  if (owner() == caller) {  // Local free.
#ifdef PROFILE
    local_frees.fetch_add(1);
#endif  // PROFILE
    return local_free_list_.Push(p) + NrRemoteObjects();
  } else {  // Remote free.
#ifdef PROFILE
    remote_frees.fetch_add(1);
#endif  // PROFILE
    return remote_free_list_.PushReturnTag(p) + NrLocalObjects();
  }
}

always_inline void Span::MoveRemoteToLocalObjects() {
  if (NrRemoteObjects() != 0) {
    int32_t actual_len = 0;
    void* objects = nullptr;
    remote_free_list_.PopAll(&objects, &actual_len);
    if (NrLocalObjects() == 0) {
      local_free_list_.SetList(objects, actual_len);
      return;
    }
    void* next;
    int32_t count = 0;
    while (objects != nullptr) {
      next = *(reinterpret_cast<void**>(objects));
      count++;
      local_free_list_.Push(objects);
      objects = next;
    }
    ScallocAssert(count == actual_len);
    LOG(kTrace, "[%d] have %d local objs", owner().tag(), NrLocalObjects());
  }
}

}  // namespace scalloc

#undef FOR_ALL_SPAN_FIELDS

#endif  // SCALLOC_SPAN_H_

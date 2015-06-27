// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_CORE_H_
#define SCALLOC_CORE_H_

#include <pthread.h>
#include <stdlib.h>

#include "arena.h"
#include "atomic_value.h"
#include "core_id.h"
#include "deque.h"
#include "globals.h"
#include "large-objects.h"
#include "lock.h"
#include "size_classes.h"
#include "span.h"

namespace scalloc {

extern int seen_memalign;


class Core {
 public:
  always_inline Core();
  always_inline void* Allocate(size_t size);
  always_inline void Free(void* p);
  always_inline void Destroy();
  always_inline void Init(core_id id);

 protected:
  typedef Stack<64> RemoteFullSpans;

  always_inline core_id id() { return id_; }

  always_inline void CheckAlignments();
  always_inline Span* GetSpan(int32_t sc);

  void* core_link_;
  core_id id_;
  Span* hot_span_[kNumClasses];
  Deque r_spans_[kNumClasses];

  uint8_t pad_[128 - ((
      sizeof(core_link_) +
      sizeof(id_) +
      sizeof(hot_span_)) % 128)];
};

#define FOR_ALL_CORE_FIELDS(V)                                                 \
  V(core_link_)                                                                \
  V(id_)                                                                       \
  V(hot_span_)                                                                 \
  V(r_spans_)                                                                  \


Core::Core() {
#ifdef DEBUG
  CheckAlignments();
#endif  // DEBUG
}


void Core::CheckAlignments() {
  // Dynamically check field alignment for 32bit.

#define CHECK_FIELD(field)                                                     \
  if ((reinterpret_cast<intptr_t>(&field) & 0x3) != 0) {                       \
    Fatal("Field " #field " not properly aligned.");                           \
  }

FOR_ALL_CORE_FIELDS(CHECK_FIELD)

#undef CHECK_FIELD
}


void Core::Init(core_id id) {
  id_ = id;
  for (int32_t i = 0 ; i < kNumClasses; i++) {
    r_spans_[i].Open(id);
  }
}


void Core::Destroy() {
  for (size_t i = 0; i < kNumClasses; i++) {
    r_spans_[i].Close();

    if (hot_span_[i] != nullptr) {
      hot_span_[i]->NewMarkFloating();
      hot_span_[i] = nullptr;
    }

    r_spans_[i].RemoveAll();
  }

  id_ = kTerminated;
}


Span* Core::GetSpan(int32_t sc) {
  Span* newspan = nullptr;
  DoubleListNode* node = nullptr;
  while ((node = r_spans_[sc].RemoveBack()) != nullptr) {
    newspan = Span::FromSpanLink(node);
    int32_t epoch = newspan->epoch();
    if (newspan->NewMarkHot(epoch)) {
      ScallocAssert(newspan->owner() == id());
      newspan->MoveRemoteToLocalObjects();
      break;
    }
    newspan = nullptr;
  }
  if (newspan == nullptr) {
    newspan = Span::New(sc, id());
  }
#if defined(SCALLOC_NO_CLEANUP_IN_FREE)
  Span* cleanup_span = nullptr;
  while ((node = r_spans_[sc].RemoveBack()) != nullptr) {
    cleanup_span = Span::FromSpanLink(node);
    const int32_t epoch = cleanup_span->epoch();
    if (cleanup_span->NrFreeObjects() == ClassToObjects[cleanup_span->size_class()]) {
      const bool success = cleanup_span->NewMarkFull(epoch);
      ScallocAssert(success);  // should always work
      Span::Delete(cleanup_span);
    }
  }
#endif  // SCALLOC_NO_CLEANUP_IN_FREE
  return newspan;
}


void* Core::Allocate(size_t size) {
  ScallocAssert(id() != kTerminated);
  const size_t sc = SizeToClass(size);
  if (UNLIKELY(hot_span_[sc] == nullptr)) {
    if (UNLIKELY(sc == 0)) {
      // Could either be allocation for size 0, or a really large object.
      if (UNLIKELY(size == 0)) {
        return nullptr;
      }
      return LargeObject::Allocate(size);
    }
    hot_span_[sc] = GetSpan(sc);
  }
  void* obj = hot_span_[sc]->Allocate();
  if (UNLIKELY(obj == nullptr)) {
    if (hot_span_[sc]->NrFreeObjects() > ClassToReuseThreshold[sc]) {
      hot_span_[sc]->MoveRemoteToLocalObjects();
      obj = hot_span_[sc]->Allocate();
      return obj;
    }

    hot_span_[sc]->NewMarkFloating();
    hot_span_[sc] = GetSpan(sc);
    if (UNLIKELY(hot_span_[sc] == nullptr)) {
      errno = ENOMEM;
      return nullptr;
    }
    obj = hot_span_[sc]->Allocate();
  }
  return obj;
}


void Core::Free(void* p) {
  ScallocAssert(id() != kTerminated);
  Span* s = Span::FromObject(p);
  if (UNLIKELY(seen_memalign != 0)) {
    p = s->AlignToBlockStart(p);
  }

  const int32_t old_epoch = s->epoch();
  core_id old_owner = s->owner();
  const int32_t size_class = s->size_class();
  const int32_t free_objects = s->Free(p, id());

  if ((old_owner.value()->id() == kTerminated) ||
      (old_owner != old_owner.value()->id())) {
    if (s->TryReviveNew(old_owner, id())) {
      old_owner = id();
      s->TryMarkFloating(old_epoch);
    }
  }


#if !defined(SCALLOC_NO_CLEANUP_IN_FREE)
  if (UNLIKELY((free_objects == ClassToObjects[size_class]) &&
      Span::IsFloatingOrReusable(old_epoch))) {
      if (s->NewMarkFull(old_epoch)) {
        if (Span::IsReusable(old_epoch)) {
          old_owner.value()->r_spans_[size_class].Remove(
              old_owner, s->SpanLink());
        }
        ScallocAssert(!Span::IsHot(s->epoch()));
        Span::Delete(s);
      }
#else
  if (false) {
#endif  // !SCALLOC_NO_CLEANUP_IN_FREE
  } else if (UNLIKELY((free_objects > ClassToReuseThreshold[size_class]) &&
             !Span::IsReusable(old_epoch))) {
      // For a terminated owner that is waiting we will still add it to the list
      // to keep the code paths simple. (Yep, that's overhead in this rare
      // case.)
      if (s->NewMarkReuse(old_epoch)) {
        old_owner.value()->r_spans_[size_class].PushFront(
            old_owner, s->SpanLink());
      }
  }
}


class GuardedCore : public Core {
 public:
  always_inline GuardedCore();
  always_inline void* Allocate(size_t size);
  always_inline void Free(void* p);

  always_inline bool InUse() { return in_use_ == 1; }
  always_inline void AnnounceNewThread() { num_threads_.fetch_add(1); }

 protected:
  always_inline void* AllocateLocked(size_t size);
  always_inline void FreeLocked(void* p);

  always_inline void Acquire() { in_use_ = 1; }
  always_inline void Release() { in_use_ = 0; }

  std::atomic<uint_fast32_t> num_threads_;

  // SW/MR.
  volatile uint_fast32_t in_use_;

  typedef SpinLock<64> Lock;

  Lock core_lock_;
};


GuardedCore::GuardedCore()
    : Core()
    , num_threads_(0)
    , in_use_(0) {
}


void* GuardedCore::Allocate(size_t size) {
  void* p;
  Acquire();
  if (LIKELY(num_threads_.load() == 1)) {
    p = Core::Allocate(size);
  } else {
    p = AllocateLocked(size);
  }
  Release();
  return p;
}


void GuardedCore::Free(void* p) {
  Acquire();
  if (LIKELY(num_threads_.load() == 1)) {
    Core::Free(p);
  } else {
    FreeLocked(p);
  }
  Release();
}


void* GuardedCore::AllocateLocked(size_t size) {
  Lock::Guard guard(core_lock_);
  return Core::Allocate(size);
}


void GuardedCore::FreeLocked(void* p) {
  Lock::Guard guard(core_lock_);
  Core::Free(p);
}

}  // namespace scalloc

#undef FOR_ALL_CORE_FIELDS

#endif  // SCALLOC_CORE_H_


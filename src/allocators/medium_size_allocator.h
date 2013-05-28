// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_MEDIUM_SIZE_ALLOCATOR_H_
#define SCALLOC_MEDIUM_SIZE_ALLOCATOR_H_

#include "allocators/half_fit.h"
#include "dlist.h"
#include "spinlock-inl.h"

class MediumSizeAllocator {
 public:
  static void InitModule();
  static bool InRange(void* p);
  static void* Allocate(size_t size);
  static void Free(void* p);
  static size_t SizeOf(void* p);
  static bool Enabled();

 private:
  static const size_t kSpanSize = 1UL << 28;  // 256MiB

  static bool enabled_;
  static GlobalSbrkAllocator arena_;
  static SpinLock lock_;
  static DList<HalfFit*> list_;

  static HalfFit* NewHalfFitSpan();
};

inline void MediumSizeAllocator::InitModule() {
  arena_.Init(kSmallSpace);
  list_.Init();
  enabled_ = true;
}

inline bool MediumSizeAllocator::Enabled() {
  return enabled_;
}

inline bool MediumSizeAllocator::InRange(void* p) {
  return arena_.InArena(p);
}

inline void* MediumSizeAllocator::Allocate(size_t size) {
  SpinLockHolder holder(&lock_);
  if (list_.Empty()) {
 FULL:
    list_.Insert(NewHalfFitSpan());
  }
  HalfFit* hf;
  void* p;
  for (DList<HalfFit*>::iterator it = list_.begin();
       it != list_.end();
       it = it->next) {
    hf = it->data;
    p = hf->Allocate(size);
    if (p) {
      break;
    }
  }
  if (!p) {
    // all spans have been full, lets get a new one
    goto FULL;
  }
  return p;
}

inline void MediumSizeAllocator::Free(void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p) & ~(kSpanSize - 1);
  HalfFit* hf = reinterpret_cast<HalfFit*>(ptr);
  hf->Free(p);

  CompilerBarrier();
  SpinLockHolder holder(&lock_);
  if (hf->Empty() && list_.Len() > 1) {
    list_.Remove(hf);
  }
}

inline HalfFit* MediumSizeAllocator::NewHalfFitSpan() {
  void* p = arena_.Allocate(kSpanSize);
  HalfFit* hf = reinterpret_cast<HalfFit*>(p);
  hf->Init(p, kSpanSize);
  return hf;
}

inline size_t MediumSizeAllocator::SizeOf(void* p) {
  return HalfFit::SizeOf(p);
}

#endif  // SCALLOC_MEDIUM_SIZE_ALLOCATOR_H_

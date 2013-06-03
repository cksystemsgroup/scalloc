// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_MEDIUM_ALLOCATOR_H_
#define SCALLOC_MEDIUM_ALLOCATOR_H_

#include "allocators/half_fit.h"
#include "dlist.h"
#include "scalloc_arenas.h"
#include "spinlock-inl.h"

namespace scalloc {

class MediumAllocator {
 public:
  static void InitModule();
  static void* Allocate(size_t size);
  static void Free(void* p);
  static size_t SizeOf(void* p);
  static bool Enabled();

 private:
  static bool enabled_;
  static SpinLock lock_;
  static DList<HalfFit*> list_;

  static HalfFit* NewHalfFitSpan();
};

inline void MediumAllocator::InitModule() {
  list_.Init();
  enabled_ = true;
}

inline bool MediumAllocator::Enabled() {
  return enabled_;
}

inline void* MediumAllocator::Allocate(size_t size) {
  SpinLockHolder holder(&lock_);
  if (list_.Empty()) {
 FULL:
    list_.Insert(NewHalfFitSpan());
  }
  HalfFit* hf;
  void* p = NULL;
  size_t cnt = 0;
  for (DList<HalfFit*>::iterator it = list_.begin();
       it != list_.end();
       it = it->next) {
    cnt++;
    hf = it->data;
    p = hf->Allocate(size);
    if (p) {
      break;
    }
  }
  if (p == NULL) {
    // all spans have been full, lets get a new one
    goto FULL;
  }
  return p;
}

inline void MediumAllocator::Free(void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p) & ~(kMediumSpanSize - 1);
  HalfFit* hf = reinterpret_cast<HalfFit*>(ptr);
  hf->Free(p);

  CompilerBarrier();
  SpinLockHolder holder(&lock_);
  if (hf->Empty() && list_.Len() > 1) {
    list_.Remove(hf);
    MediumArena.Free(reinterpret_cast<void*>(hf), kMediumSpanSize);
  }
}

inline HalfFit* MediumAllocator::NewHalfFitSpan() {
  void* p = MediumArena.Allocate(kMediumSpanSize);
  HalfFit* hf = reinterpret_cast<HalfFit*>(p);
  hf->Init(kMediumSpanSize);
  return hf;
}

inline size_t MediumAllocator::SizeOf(void* p) {
  return HalfFit::SizeOf(p);
}

}  // namespace scalloc

#endif  // SCALLOC_MEDIUM_ALLOCATOR_H_

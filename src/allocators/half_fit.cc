// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/half_fit.h"

#include "common.h"

//
// In memory doubly-linked list.
//

void HalfFit::ListRemove(ListHeader* elem, const size_t sc) {
  if (flist_[sc] == elem) {
    flist_[sc] = flist_[sc]->next;
  }
  if (elem->prev != NULL) {
    elem->prev->next = elem->next;
  }
  if (elem->next != NULL) {
    elem->next->prev = elem->prev;
  }
}

void HalfFit::ListAdd(ListHeader* elem, const size_t sc) {
  elem->prev = NULL;
  elem->next = flist_[sc];
  if (flist_[sc] != NULL) {
    flist_[sc]->prev = elem;
  }
  flist_[sc] = elem;
}

HalfFit::ListHeader* HalfFit::ListGet(const size_t sc) {
  ListHeader* elem = flist_[sc];
  if (elem != NULL) {
    ListRemove(elem, sc);
  }
  return elem;
}

//
// Actual Half-Fit.
//

void HalfFit::Init(const size_t len) {
  // init fields; lock_ has a constructor, so it's fine.
  for (size_t i = 0; i < kClasses; i++) {
    flist_[i] = NULL;
  }
  used_ = 0;

  // calculate the actually usable area for half-fit
  size_t usable_len = len;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
  // acount for the actual object (this) and a dummy header.
  const size_t span_hdr_sz = sizeof(this) +
                             sizeof(ObjectHeader);
  ptr += span_hdr_sz;
  usable_len -= span_hdr_sz;
  // account for properly aligned (to kLargestSize) memory
  const size_t rest = kLargestSize - (ptr % kLargestSize);
  ptr += rest;
  usable_len -= rest;
  // account for a dummy trailer
  usable_len -= kLargestSize;

  // set the dummy header
  ObjectHeader* oh = reinterpret_cast<ObjectHeader*>(
      ptr - sizeof(ObjectHeader));
  oh->Set(true, -1 /*unused*/);

  // split up the available memory into kLargestSize chunks and add them to the
  // largest size class
  for (size_t i = 0; i < (usable_len / kLargestSize); i++) {
    oh = reinterpret_cast<ObjectHeader*>(ptr);
    SetHeaderThenFooter(oh, false, kClasses - 1);
    ListAdd(GetListHeader(oh), oh->sc);
    ptr += kLargestSize;
  }

  // set the dummy trailer
  oh = reinterpret_cast<ObjectHeader*>(ptr);
  oh->Set(true, -1 /*unused*/);
}


void* HalfFit::Allocate(const size_t size) {
  SpinLockHolder holder(&lock_);
  CompilerBarrier();

  used_++;
  size_t sc = SizeToClass(size);
  ListHeader* elem = NULL;
  for (size_t i = sc; i < kClasses; i++) {
    elem = ListGet(i);
    if (elem != NULL) {
      ObjectHeader* oh = GetObjectHeader(elem);
      if (oh->sc != i) {
        ErrorOut("something is wrong");
      }
      oh = Split(oh, i - sc);
      elem = GetListHeader(oh);
      break;
    }
  }
  return elem;
}

HalfFit::ObjectHeader* HalfFit::Split(ObjectHeader* oh, size_t level) {
  if (level == 0) {
    SetHeaderThenFooter(oh, true, oh->sc);
    return oh;
  }
  ScallocAssert(oh->sc != 0, "size class 0 cannot be split");
  const size_t new_sc = oh->sc - 1;
  SetHeaderThenFooter(oh, false, new_sc);
  ObjectHeader* right = reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh) + ClassToSize(new_sc));
  SetHeaderThenFooter(right, false, new_sc);
  ListAdd(GetListHeader(right), new_sc);
  return Split(oh, level - 1);
}

void HalfFit::Free(void* p) {
  SpinLockHolder holder(&lock_);
  CompilerBarrier();

  used_--;
  ListHeader* lh = reinterpret_cast<ListHeader*>(p);
  ObjectHeader* oh = GetObjectHeader(lh);
  SetHeaderThenFooter(oh, false, oh->sc);
  TryMerge(oh);
}

// We want to merge (do coalescing) on neighbored blockes.  Besides making sure
// that we actualy can coalesce (size class, used), we also have to make sure
// that we only coalesce blocks that would then result in a merged block that is
// corectly aligned.  Otherwise one cannot guarantee to reach the starting state
// again.
void HalfFit::TryMerge(ObjectHeader* oh) {
  const size_t new_sc = oh->sc + 1;
  ObjectHeader* lh = GetLeftHeader(oh);
  if (lh &&
      oh->sc < (kClasses - 1) &&
      lh->sc == oh->sc &&
      (((1UL << (new_sc + kMinShift)) - 1) &
          reinterpret_cast<uintptr_t>(lh)) == 0) {
    ListRemove(GetListHeader(lh), lh->sc);
    SetHeaderThenFooter(lh, false, new_sc);
    TryMerge(lh);
    return;
  }
  ObjectHeader* rh = GetRightHeader(oh);
  if (rh &&
      (oh->sc < (kClasses - 1)) &&
      (rh->sc == oh->sc) &&
      ((((1UL << (new_sc + kMinShift)) - 1) &
          reinterpret_cast<uintptr_t>(oh)) == 0)) {
    ListRemove(GetListHeader(rh), rh->sc);
    SetHeaderThenFooter(oh, false, new_sc);
    TryMerge(oh);
    return;
  }
  
  ListAdd(GetListHeader(oh), oh->sc);
}

bool HalfFit::Empty() {
  SpinLockHolder holder(&lock_);
  CompilerBarrier();

  return used_ == 0;
}

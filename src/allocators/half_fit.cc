// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/half_fit.h"

#include "common.h"
#include "log.h"

void HalfFit::Init(void* block, size_t len) {
  for (size_t i = 0; i < kClasses; i++) {
    flist_[i] = NULL;
  }
  used_ = 0;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(block);
  const size_t span_hdr_sz = sizeof(this) +
                             sizeof(ObjectHeader);  // including dummy header
  ptr += span_hdr_sz;
  len -= span_hdr_sz;
  const size_t rest = kLargestSize - (ptr % kLargestSize);
  ptr += rest;
  len -= rest;
  len -= kLargestSize;  // dummy trailer
  ObjectHeader* oh = reinterpret_cast<ObjectHeader*>(
      ptr - sizeof(ObjectHeader));
  oh->used = true;  // set the dummy header
  oh->sc = 42;
  for (size_t i = 0; i < (len / kLargestSize); i++) {
    oh = reinterpret_cast<ObjectHeader*>(ptr);
    oh->Set(false, kClasses - 1);
    GetFooter(oh)->Set(false, oh->sc);
    ListAdd(GetListHeader(oh), oh->sc);
    ptr += kLargestSize;
  }
  oh = reinterpret_cast<ObjectHeader*>(ptr);
  oh->used = true;  // set the dummy trailer
  oh->sc = 42;
}

void HalfFit::ListRemove(ListHeader* elem, const size_t sc) {
  if (elem->prev) {
    elem->prev->next = elem->next;
  }
  if (elem->next) {
    elem->next->prev = elem->prev;
  }
  if (flist_[sc] == elem) {
    flist_[sc] = elem->next;
  }
}

void HalfFit::ListAdd(ListHeader* elem, const size_t sc) {
  elem->next = flist_[sc];
  elem->prev = NULL;
  if (flist_[sc] != NULL) {
    flist_[sc]->prev = elem;
  }
  flist_[sc] = elem;
}

HalfFit::ListHeader* HalfFit::ListGet(const size_t sc) {
  if (flist_[sc] == NULL) {
    return NULL;
  }
  ListHeader* elem = flist_[sc];
  flist_[sc] = flist_[sc]->next;
  return elem;
}

void* HalfFit::Allocate(const size_t size) {
  SpinLockHolder holder(&lock_);
  size_t sc = SizeToSizeClass(size);
  used_++;
  ListHeader* elem = NULL;
  for (size_t i = sc; i < kClasses; i++) {
    elem = ListGet(i);
    if (elem != NULL) {
      ObjectHeader* oh = Split(GetObjectHeader(elem), i - sc);
      elem = GetListHeader(oh);
    }
  }
  return elem;
}

HalfFit::ObjectHeader* HalfFit::Split(ObjectHeader* oh, size_t level) {
  if (level == 0) {
    return oh;
  }
  oh->Set(false, oh->sc - 1);
  GetFooter(oh)->Set(false, oh->sc);
  ObjectHeader* right = reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh) + SizeClassToSize(oh->sc));
  right->Set(false, oh->sc);
  GetFooter(right)->Set(false, oh->sc);
  ListAdd(GetListHeader(right), right->sc);
  return Split(oh, level - 1);
}

void HalfFit::Free(void* p) {
  SpinLockHolder holder(&lock_);
  used_--;
  ListHeader* lh = reinterpret_cast<ListHeader*>(p);
  TryMerge(GetObjectHeader(lh));
}

void HalfFit::TryMerge(ObjectHeader* oh) {
  ObjectHeader* lh = GetLeftHeader(oh);
  if (lh &&
      oh->sc < (kClasses - 1) &&
      lh->sc == oh->sc &&
      (((1UL << (lh->sc + kMinShift)) - 1) &
          reinterpret_cast<uintptr_t>(lh)) == 0) {
    ListRemove(GetListHeader(lh), lh->sc);
    oh = lh;
    oh->Set(false, oh->sc + 1);
    return TryMerge(oh);
  }
  ObjectHeader* rh = GetRightHeader(oh);
  if (rh &&
      oh->sc < (kClasses - 1) &&
      rh->sc == oh->sc &&
      (((1UL << (oh->sc + kMinShift)) - 1) &
          reinterpret_cast<uintptr_t>(oh)) == 0) {
    ListRemove(GetListHeader(rh), rh->sc);
    oh->Set(false, oh->sc + 1);
    GetFooter(oh)->Set(false, oh->sc);
    return TryMerge(oh);
  }
  ListAdd(GetListHeader(oh), oh->sc);
}

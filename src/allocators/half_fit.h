// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "spinlock-inl.h"

class HalfFit {
 public:
  static size_t SizeOf(void* p);

  void Init(void* block, size_t len);
  void* Allocate(const size_t size);
  void Free(void* p);
  bool Empty();

 private:

  struct ObjectHeader {
    bool used;
    size_t sc;

    void Set(bool used, size_t sc) {
      this->used = used;
      this->sc = sc;
    }
  };

  struct ListHeader {
    ListHeader* prev;
    ListHeader* next;
  };

  static const size_t kMinShift = 10;
  static const size_t kMinSize = 1UL << kMinShift;
  static const size_t kClasses = 12;
  static const size_t kLargestSize = 1UL << (kMinShift + kClasses - 1);

  static size_t SizeToSizeClass(size_t size);
  static size_t SizeClassToSize(size_t sc);
  static ObjectHeader* GetObjectHeader(ListHeader* lh);

  SpinLock lock_;
  ListHeader* flist_[kClasses];
  size_t used_;

  void ListRemove(ListHeader* elem, const size_t sc);
  void ListAdd(ListHeader* elem, const size_t sc);
  ListHeader* ListGet(const size_t sc);
  ObjectHeader* GetLeftHeader(ObjectHeader* oh);
  ObjectHeader* GetRightHeader(ObjectHeader* oh);
  ObjectHeader* GetFooter(ObjectHeader* oh);
  ListHeader* GetListHeader(ObjectHeader* oh);
  ObjectHeader* Split(ObjectHeader* oh, size_t level);
  void TryMerge(ObjectHeader* oh);
};

inline size_t HalfFit::SizeOf(void* p) {
  ObjectHeader* oh = GetObjectHeader(reinterpret_cast<ListHeader*>(p));
  return SizeClassToSize(oh->sc);
}

inline size_t HalfFit::SizeToSizeClass(size_t size) {
  size += 2 * sizeof(ObjectHeader);
  return Log2(size - 1) + 1 - kMinShift;
}

inline size_t HalfFit::SizeClassToSize(size_t sc) {
  return 1UL << (sc + kMinShift);
}

inline HalfFit::ObjectHeader* HalfFit::GetObjectHeader(ListHeader* lh) {
  return reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(lh) - sizeof(ObjectHeader));
}

inline  HalfFit::ListHeader* HalfFit::GetListHeader(ObjectHeader* oh) {
  return reinterpret_cast<ListHeader*>(
      reinterpret_cast<uintptr_t>(oh) + sizeof(ObjectHeader));
}

inline HalfFit::ObjectHeader* HalfFit::GetLeftHeader(ObjectHeader* oh) {
  oh = reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh) - sizeof(ObjectHeader));
  if (oh->used == false) {
    return reinterpret_cast<ObjectHeader*>(
        reinterpret_cast<uintptr_t>(oh)
        - SizeClassToSize(oh->sc)
        + sizeof(ObjectHeader));
  }
  return NULL;
}

inline HalfFit::ObjectHeader* HalfFit::GetRightHeader(ObjectHeader* oh) {
  oh = reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh) + SizeClassToSize(oh->sc));
  if (oh->used == false) {
    return oh;
  }
  return NULL;
}

inline HalfFit::ObjectHeader* HalfFit::GetFooter(ObjectHeader* oh) {
  return reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh)
      + SizeClassToSize(oh->sc)
      - sizeof(ObjectHeader));
}

inline bool HalfFit::Empty() {
  SpinLockHolder holder(&lock_);
  return used_ == 0;
}

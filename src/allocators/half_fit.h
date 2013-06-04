// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

// Implementing the Half-Fit algorithm from:
//
// T. Ogasawara. 1995. An algorithm with constant execution time for dynamic
// storage allocation. In Proc. International Workshop on Real-Time Computing
// Systems and Applications (RTCSA '95). IEEE.

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "spinlock-inl.h"

// Half-fit style allocation.
//
// Memory layout:
// +------+--------------+----------+---------------+
// | this | dummy header | half-fit | dummy trailer |
// +------+--------------+----------+---------------+
//                       ^- aligned to largest size class
// Half-Fit layout:
// +--------------+-----------------------+--------------+
// | ObjectHeader | Payload or ListHeader | ObjectFooter |
// +--------------+-----------------------+--------------+
//                ^- 16 bytes aligned (min for C++)
class HalfFit {
 public:
  static size_t SizeOf(void* p);

  void Init(const size_t len);
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
  } __attribute__((aligned(16)))___;

  struct ListHeader {
    ListHeader* prev;
    ListHeader* next;
  };

  static const size_t kMinShift = 10;
  static const size_t kMinSize = 1UL << kMinShift;
  static const size_t kClasses = 12;
  static const size_t kLargestSize = 1UL << (kMinShift + kClasses - 1);

  static size_t SizeToClass(size_t size);
  static size_t ClassToSize(size_t sc);
  static ObjectHeader* GetObjectHeader(ListHeader* lh);
  static ObjectHeader* GetLeftHeader(ObjectHeader* oh);
  static ObjectHeader* GetRightHeader(ObjectHeader* oh);
  static ObjectHeader* GetFooter(ObjectHeader* oh);
  static ListHeader* GetListHeader(ObjectHeader* oh);
  static void SetHeaderThenFooter(ObjectHeader* oh, bool used, size_t sc);

  SpinLock lock_;
  ListHeader* flist_[kClasses];
  size_t used_;

  void ListRemove(ListHeader* elem, const size_t sc);
  void ListAdd(ListHeader* elem, const size_t sc);
  ListHeader* ListGet(const size_t sc);
  ObjectHeader* Split(ObjectHeader* oh, size_t level);
  void TryMerge(ObjectHeader* oh);
};

inline size_t HalfFit::SizeOf(void* p) {
  ObjectHeader* oh = GetObjectHeader(reinterpret_cast<ListHeader*>(p));
  return ClassToSize(oh->sc);
}

inline size_t HalfFit::SizeToClass(size_t size) {
  size += 2 * sizeof(ObjectHeader);
  return Log2(size - 1) + 1 - kMinShift;
}

inline size_t HalfFit::ClassToSize(size_t sc) {
  return 1UL << (sc + kMinShift);
}

inline HalfFit::ObjectHeader* HalfFit::GetObjectHeader(ListHeader* lh) {
  return reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(lh) - sizeof(ObjectHeader));
}

inline HalfFit::ObjectHeader* HalfFit::GetLeftHeader(ObjectHeader* oh) {
  oh = reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh) - sizeof(ObjectHeader));
  if (oh->used == false) {
    return reinterpret_cast<ObjectHeader*>(
        reinterpret_cast<uintptr_t>(oh)
        - ClassToSize(oh->sc)
        + sizeof(ObjectHeader));
  }
  return NULL;
}

inline HalfFit::ObjectHeader* HalfFit::GetRightHeader(ObjectHeader* oh) {
  oh = reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh) + ClassToSize(oh->sc));
  if (oh->used == false) {
    return oh;
  }
  return NULL;
}

inline HalfFit::ObjectHeader* HalfFit::GetFooter(ObjectHeader* oh) {
  return reinterpret_cast<ObjectHeader*>(
      reinterpret_cast<uintptr_t>(oh)
      + ClassToSize(oh->sc)
      - sizeof(ObjectHeader));
}

inline  HalfFit::ListHeader* HalfFit::GetListHeader(ObjectHeader* oh) {
  return reinterpret_cast<ListHeader*>(
      reinterpret_cast<uintptr_t>(oh) + sizeof(ObjectHeader));
}

inline void HalfFit::SetHeaderThenFooter(ObjectHeader* oh,
                                         bool used,
                                         size_t sc) {
  oh->Set(used, sc);
  GetFooter(oh)->Set(used, sc);
}

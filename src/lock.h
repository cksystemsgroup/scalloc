// Copyright (c) 2014, the Scal Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCK_H_
#define SCALLOC_LOCK_H_

#include <atomic>

#include "log.h"
#include "platform/globals.h"

template<typename LockType>
class LockHolder;

template<int PAD = 0>
class SpinLock {
 public:
  typedef LockHolder<SpinLock<PAD> > Guard;

  always_inline SpinLock() : lock_(0) {}

  always_inline void Lock() {
    while (!TryLock()) {
      __asm__("PAUSE");
    }
  }

  always_inline bool TryLock() { return lock_.exchange(1) == 0; }
  always_inline void Unlock() { lock_.store(0); }

 private:
  std::atomic<uint_fast32_t> lock_;
  uint8_t _padding[(PAD != 0) ?  (PAD - sizeof(lock_)) : 0];
};


template<typename LockType>
class LockHolder {
 public:
  always_inline explicit LockHolder(LockType& lock) : lock_(lock) {  // NOLINT
    lock_.Lock();
  }

  always_inline ~LockHolder() {
    lock_.Unlock();
  }

 private:
  LockType& lock_;
};

#endif  // SCALLOC_LOCK_H_

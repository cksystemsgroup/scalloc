// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_SPINLOCK_H_
#define SCALLOC_SPINLOCK_H_

#include <stdint.h>

#include "assert.h"

enum LinkerInitialized {
  LINKER_INITIALIZED
};

// Basic spinlock (busy loop)
class SpinLock {
 public:
  SpinLock() : lockword_(kSpinLockFree) { }

  // We use this special constructor to handle static variables which can be
  // used during global constructor time. Works because linker initializes
  // lockword_ with 0 (= kSpinLockFree)
  explicit SpinLock(LinkerInitialized not_used) { }

  inline void Lock() {
    while (__sync_lock_test_and_set(&lockword_, kSpinLockHeld)) { }
  }

  inline void Unlock() {
    __sync_lock_release(&lockword_);
  }

  inline void Reset() {
    lockword_ = 0;
  }

 private:
  enum { kSpinLockFree = 0 };
  enum { kSpinLockHeld = 1 };

  volatile uint64_t lockword_;

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(SpinLock);
};

#define LockScope(lock)                                                        \
  SpinLockHolder holder(&lock);                                                \
  __asm__ __volatile__("" : : : "memory");

class SpinLockHolder {
 public:
  inline explicit SpinLockHolder(SpinLock *lock) : lock_(lock) {
    lock_->Lock();
  }

  inline ~SpinLockHolder() {
    lock_->Unlock();
  }

 private:
  SpinLock* lock_;

  DISALLOW_ALLOCATION();
  DISALLOW_IMPLICIT_CONSTRUCTORS(SpinLockHolder);
};

#endif  // SCALLOC_SPINLOCK_H_

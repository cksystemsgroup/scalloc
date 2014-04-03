// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_FAST_LOCK_H_
#define SCALLOC_FAST_LOCK_H_

#ifdef __linux__

#include <linux/futex.h>
#include <sys/syscall.h>


inline int futex_wait(int* uaddr, int val1) {
  return syscall(SYS_futex, uaddr, FUTEX_WAIT, val1, NULL, NULL, 0);
}


inline int futex_wake(int* uaddr, int val1) {
  return syscall(SYS_futex, uaddr, FUTEX_WAKE, val1, NULL, NULL, 0);
}


// Futex-based lock as described in "Futexes Are Tricky" by Ulrich Drepper.
class FastLock {
 public:
  FastLock() : lockword_(0) {}

  inline void Init() {
    lockword_ = 0;
  }

  inline void Lock() {
    if (__sync_bool_compare_and_swap(&lockword_, 0 , 1)) {
      // uncontended
      return;
    }
    do {
      if (lockword_ == 2 || __sync_bool_compare_and_swap(&lockword_, 1, 2)) {
        // contended
        futex_wait((int*)&lockword_, 2);
      }
    } while (!__sync_bool_compare_and_swap(&lockword_, 0, 2));
    // 0->2 successful
  }

  inline void Unlock() {
    if (__sync_fetch_and_sub(&lockword_, 1) != 1) {
      // lockword_ was 2.
      lockword_ = 0;  // Tricky: Can interleave with Lock() in do loop.
      futex_wake((int*)&lockword_, 1);
    }
  }

 private:
  static const int kFree = 0;
  static const int kUncontended = 1;
  static const int kContended = 2;

  int64_t lockword_;
  uint64_t __padding[7];  // Pad for 64 bytes cache line.
};


#define FastLockScope(lock)                                                    \
  FastLockHolder holder(&lock);                                                \
  __asm__ __volatile__("" : : : "memory");


class FastLockHolder {
 public:
  inline explicit FastLockHolder(FastLock* lock) : lock_(lock) {
    lock_->Lock();
  }

  inline ~FastLockHolder() {
    lock_->Unlock();
  }

 private:
  FastLock* lock_;

  DISALLOW_ALLOCATION();
  DISALLOW_IMPLICIT_CONSTRUCTORS(FastLockHolder);
};

#endif  // __linux__

#endif  // SCALLOC_FAST_LOCK_H_


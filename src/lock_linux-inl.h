// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCK_LINUX_INL_H_
#define SCALLOC_LOCK_LINUX_INL_H_

#ifdef __linux__

#include <linux/futex.h>
#include <sys/syscall.h>

class FutexLock {
 public:
  FutexLock() : lockword_(0) {}

  inline void Init() {
    lockword_ = 0;
    sleep_cnt_ = 0;
  }

  inline void Lock() {
    if (__sync_bool_compare_and_swap(&lockword_, kFree , kUncontended)) {
      // uncontended
      return;
    }
    do {
      if (lockword_ == kContended ||
          __sync_bool_compare_and_swap(&lockword_, kUncontended, kContended)) {
        // contended
        __sync_fetch_and_add(&sleep_cnt_, 1);
        futex_wait(reinterpret_cast<int*>(&lockword_), kContended);
        __sync_fetch_and_sub(&sleep_cnt_, 1);
      }
    } while (!__sync_bool_compare_and_swap(&lockword_, kFree, kContended));
    // 0->2 successful
  }

  inline void Unlock() {
    if (__sync_fetch_and_sub(&lockword_, 1) != kUncontended) {
      // lockword_ was kContended.
      lockword_ = kFree;  // Tricky: Can interleave with Lock() in do loop.
      futex_wake(reinterpret_cast<int*>(&lockword_), 1);  // Wake up 1 thread.
    }
  }

  inline uint64_t SleepingThreads() {
    return sleep_cnt_;
  }

 private:
  static const int kFree = 0;
  static const int kUncontended = 1;
  static const int kContended = 2;

  int64_t lockword_;
  uint64_t __padding1[7];  // Pad for 64 bytes cache line.
  uint64_t sleep_cnt_;
  uint64_t __padding2[7];  // Pad for 64 bytes cache line.

  inline int futex_wait(int* uaddr, int val1) {
    return syscall(SYS_futex, uaddr, FUTEX_WAIT, val1, NULL, NULL, 0);
  }

  inline int futex_wake(int* uaddr, int val1) {
    return syscall(SYS_futex, uaddr, FUTEX_WAKE, val1, NULL, NULL, 0);
  }
};

#endif  // __linux__
#endif  // SCALLOC_LOCK_LINUX_INL_H_


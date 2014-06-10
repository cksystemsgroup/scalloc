// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCK_LINUX_INL_H_
#define SCALLOC_LOCK_LINUX_INL_H_

#ifdef __linux__

#include <linux/futex.h>
#include <sys/syscall.h>

#include <atomic>

// Implements mutex3 described in ``Futesxes Are Tricky'' by Ulrich Drepper.
class FutexLock {
 public:
  FutexLock() {}

  inline void Init() {
    //lock_.store(kFree);
    lockword_ = 0;
    waiting_.store(0);
  }

  inline void Lock() {
    if (lockword_ == kFree && 
        __sync_bool_compare_and_swap(&lockword_, kFree , kUncontended)) {
      // uncontended
      return;
    }
    do {
      if (lockword_ == kContended ||
          __sync_bool_compare_and_swap(&lockword_, kUncontended, kContended)) {
        // contended
        waiting_.fetch_add(1, std::memory_order_relaxed);
        futex_wait(&lockword_, kContended);
        waiting_.fetch_sub(1, std::memory_order_relaxed);
      }
    } while (!__sync_bool_compare_and_swap(&lockword_, kFree, kContended));
    // 0->2 successful
  }

  inline void Unlock() {
    if (__sync_fetch_and_sub(&lockword_, 1) != kUncontended) {
      // lockword_ was kContended.
      lockword_ = kFree;  // Tricky: Can interleave with Lock() in do loop.
      futex_wake(&lockword_, 1);  // Wake up 1 thread.
    }
  }

  inline uint_fast64_t Waiting() {
    return waiting_.load(std::memory_order_relaxed);
  }

 private:
  static const int_fast8_t kFree = 0;
  static const int_fast8_t kUncontended = 1;
  static const int_fast8_t kContended = 2;

  int_fast8_t lockword_;
  std::atomic<uint_fast64_t> waiting_;

  inline int futex_wait(int_fast8_t* uaddr, int val1) {
    return syscall(SYS_futex, (int*)uaddr, FUTEX_WAIT, val1, NULL, NULL, 0);
  }

  inline int futex_wake(int_fast8_t* uaddr, int val1) {
    return syscall(SYS_futex, (int*)uaddr, FUTEX_WAKE, val1, NULL, NULL, 0);
  }
};

#endif  // __linux__
#endif  // SCALLOC_LOCK_LINUX_INL_H_


// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCK_UTILS_INL_H_
#define SCALLOC_LOCK_UTILS_INL_H_

#include "spinlock-inl.h"

#ifdef __linux__

#include "lock_linux-inl.h"

typedef FutexLock Lock;

#else

#include <pthread.h>
#include <atomic>

class PthreadLock {
 public:
  inline void Init() {
    pthread_mutex_init(&mutex_, NULL);
    waiting_ = 0;
  }

  inline void Lock() {
    if (pthread_mutex_trylock(&mutex_) != 0) {
      waiting_.fetch_add(1, std::memory_order_relaxed);
      pthread_mutex_lock(&mutex_);
      waiting_.fetch_sub(1, std::memory_order_relaxed);
    }
  }

  inline void Unlock() {
    pthread_mutex_unlock(&mutex_);
  }

  inline uint_fast64_t Waiting() {
    return waiting_.load(std::memory_order_relaxed);
    return 0;
  }

 private:
  pthread_mutex_t mutex_;
  std::atomic<uint_fast64_t> waiting_;
};

typedef PthreadLock Lock;

#endif  // __linux__


class LockHolder {
 public:
  inline explicit LockHolder(Lock* lock) : lock_(lock) {
    lock_->Lock();
  }

  inline ~LockHolder() {
    lock_->Unlock();
  }

 private:
  Lock* lock_;

  DISALLOW_ALLOCATION();
  DISALLOW_IMPLICIT_CONSTRUCTORS(LockHolder);
};


#define LockScope(lock)                                                        \
  LockHolder holder(&lock);                                                    \
  __asm__ __volatile__("" : : : "memory");


#endif  // SCALLOC_LOCK_UTILS_INL_H_


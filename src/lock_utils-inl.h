// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCK_UTILS_INL_H_
#define SCALLOC_LOCK_UTILS_INL_H_

#ifdef __linux__

#include "lock_linux-inl.h"

typedef FutexLock Lock;

#else

#include <pthread.h>

class PthreadLock {
 public:
  inline void Init() {
    pthread_mutex_init(&mutex, NULL);
  }

  inline void Lock() {
    pthread_mutex_lock(&mutex_);
  }

  inline void Unlock() {
    pthread_mutex_unlock(&mutex_);
  }

 private:
  pthread_mutex_t mutex_;
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


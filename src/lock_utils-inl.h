// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LOCK_UTILS_INL_H_
#define SCALLOC_LOCK_UTILS_INL_H_

#include <pthread.h>

#define LockScopePthread(lock)                                                 \
  PthreadMutexHolder holder(&lock);                                            \
  __asm__ __volatile__("" : : : "memory");


class PthreadMutexHolder {
 public:
  inline explicit PthreadMutexHolder(pthread_mutex_t* mutex) : mutex_(mutex) {
    pthread_mutex_lock(mutex_);
  }

  ~PthreadMutexHolder() {
    pthread_mutex_unlock(mutex_);
  }

 private:
  pthread_mutex_t* mutex_;

  DISALLOW_ALLOCATION();
  DISALLOW_IMPLICIT_CONSTRUCTORS(PthreadMutexHolder);
};

#endif  // SCALLOC_LOCK_UTILS_INL_H_


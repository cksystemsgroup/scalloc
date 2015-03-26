// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PLATFORM_PTHREAD_INTERCEPT_H_
#define SCALLOC_PLATFORM_PTHREAD_INTERCEPT_H_

#include <pthread.h>

typedef void* (*StartFunc)(void*);
typedef int (*PthreadCreateFunc)(pthread_t*, pthread_attr_t*, StartFunc, void*);

struct ScallocStartArgs {
  ScallocStartArgs(StartFunc real_start, void* real_args)
      : real_start(real_start)
      , real_args(real_args) { }

  StartFunc real_start;
  void* real_args;
};

extern "C" void* scalloc_thread_start(void* scalloc_start_args);

#endif  // SCALLOC_PLATFORM_PTHREAD_INTERCEPT_H_

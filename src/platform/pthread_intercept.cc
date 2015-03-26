// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "platform/pthread_intercept.h"

#include <dlfcn.h>
#include <stdlib.h>

extern "C" int pthread_create(
    pthread_t* thread, const pthread_attr_t* attr, StartFunc start, void* arg) {
  static PthreadCreateFunc real_create = NULL;
  if (!real_create) {
    real_create = reinterpret_cast<PthreadCreateFunc>(
        dlsym(RTLD_NEXT, "pthread_create"));
  }
  return real_create(
      thread,
      const_cast<pthread_attr_t*>(attr),
      scalloc_thread_start,
      new ScallocStartArgs(start, arg));
}

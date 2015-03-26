// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "platform/pthread_intercept.h"

#include <dlfcn.h>
#include <stdlib.h>

#undef pthread_create

extern "C" int pthread_create(
    pthread_t* thread, pthread_attr_t* attr, StartFunc start, void* arg) {
  static PthreadCreateFunc real_create = NULL;
  int rc;
  if (!real_create) {
    real_create = reinterpret_cast<PthreadCreateFunc>(
        dlsym(RTLD_NEXT, "pthread_create"));
  }

  ScallocStartArgs* scalloc_args = new ScallocStartArgs();
  scalloc_args->real_args = arg;
  scalloc_args->real_start = start;
  rc = real_create(thread, attr, scalloc_thread_start, scalloc_args);
  return rc;
}

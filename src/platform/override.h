// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PLATFORM_OVERRIDE_H_
#define SCALLOC_PLATFORM_OVERRIDE_H_

#if defined(__linux__)
#include "platform/override_gcc_weak.h"
#include "platform/pthread_intercept.h"

#elif defined(__APPLE__)
#include "platform/override_osx.h"
#include "platform/pthread_intercept.h"

#else
#error unsupported lib/OS.

#endif

#endif  // SCALLOC_PLATFORM_OVERRIDE_H_


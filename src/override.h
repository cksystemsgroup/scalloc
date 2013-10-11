// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_OVERRIDE_H_
#define SCALLOC_OVERRIDE_H_

static void ReplaceSystemAlloc();

#if defined(__APPLE__)
#include "override_osx.h"

#elif defined(__GNUC__)
#include "override_gcc_weak.h"

#else
#error unsupported lib/OS.

#endif

#endif  // SCALLOC_OVERRIDE_H_
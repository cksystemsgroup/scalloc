// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "runtime_vars.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"

RuntimeVars RuntimeVars::vars_;

void RuntimeVars::InitModule() {
  long ret;  // NOLINT
  if ((ret = sysconf(_SC_PAGESIZE)) == -1) {
    if (errno == 0) {
      ErrorOut("sysconf: _SC_PAGESIZE not supported");
    } else {
      ErrorOut("sysconf() failed");
    }
  }
  vars_.system_page_size_ = static_cast<size_t>(ret);

  if ((ret = sysconf(_SC_NPROCESSORS_CONF)) == -1) {
    if (errno == 0) {
      ErrorOut("sysconf: _SC_NPROCESSORS_CONF not supported");
    } else {
      ErrorOut("sysconf() failed");
    }
  }
  vars_.number_cpus_ = static_cast<size_t>(ret);
}

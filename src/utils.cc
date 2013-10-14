// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "utils.h"

#include <errno.h>
#include <unistd.h>

#include "common.h"

namespace scalloc {
namespace utils {

size_t Cpus() {
  static size_t cpus = 0;
  if (cpus == 0) {
    const int64_t ret = sysconf(_SC_NPROCESSORS_CONF);
    if (ret == -1) {
      if (errno == 0) {
        Fatal("sysconf: _SC_NPROCESSORS_CONF not supported");
      } else {
        Fatal("sysconf(_SC_NPROCESSORS_CONF) failed");
      }
    }
    cpus = static_cast<size_t>(ret);
  }
  return cpus;
}

}  // namespace utils
}  // namespace scalloc

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "utils.h"

#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "scalloc_assert.h"

namespace {

const size_t kMaxSizet = static_cast<size_t>(-1);

inline void Swap(size_t& a, size_t& b) {
  if (&a == &b) {
    return;
  }
  a ^= b;
  b ^= a;
  a ^= b;
}

inline size_t Gcd(size_t a, size_t b) {
  if (b < a) {
    Swap(a, b);
  }

  size_t t;
  while (b != 0) {
    t = b;
    b = a % b;
    a = t;
  }
  return a;
}

}  // namespace

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


size_t Parallelism() {
  static size_t parallelism = kMaxSizet;
  if (parallelism == kMaxSizet) {
#ifdef MAX_PARALLELISM
    parallelism = MAX_PARALLELISM;
#endif  // MAX_PARALLELISM
    if (Cpus() < parallelism) {
      parallelism = Cpus();
    }
  }
  return parallelism;
}

}  // namespace utils
}  // namespace scalloc

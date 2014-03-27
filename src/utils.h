// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_UTILS_H_
#define SCALLOC_UTILS_H_

#include <stddef.h>  // size_t

namespace scalloc {
namespace utils {

inline size_t PadSize(size_t size, size_t multiple) {
  return (size + multiple - 1) / multiple * multiple;
}


#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
static const char log_table[256] = {
  -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
  LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};


// base-2 logarithm of 32-bit integers
inline int Log2(size_t v) {
  unsigned int t, tt, r;  // temp vars
  if ((tt = (v >> 16))) {
    r =  (t = (tt >> 8)) ? 24 + log_table[t] : 16 + log_table[tt];
  } else {
    r =  (t = (v >> 8)) ? 8 + log_table[t] : log_table[v];
  }
  return r;
}


inline bool IsPowerOfTwo(size_t num) {
  return (num != 0) && ((num & (num - 1)) == 0);
}


size_t Cpus();

size_t Parallelism();

}  // namespace utils
}  // namespace scalloc

#endif  // SCALLOC_UTILS_H_

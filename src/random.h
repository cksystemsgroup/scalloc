// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_RANDOM_H_
#define SCALLOC_RANDOM_H_

#include <stdint.h>

inline uint64_t rdtsc(void) {
  unsigned int hi, lo;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t) lo) | (((uint64_t) hi) << 32);
}

inline uint64_t hwrand() {
  return (rdtsc() >> 6);
}

#endif  // SCALLOC_RANDOM_H_

// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_UTILS_H_
#define SCALLOC_UTILS_H_

#include <sys/mman.h>

#include "globals.h"
#include "platform/globals.h"
#include "log.h"

namespace scalloc {

always_inline size_t PadSize(size_t size, size_t multiple) {
  return (size + multiple - 1) / multiple * multiple;
}


always_inline void* SystemMmapGuided(void* hint, size_t size) {
  const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  const int prot = PROT_READ | PROT_WRITE;
  void* p = mmap(hint, size, prot, flags, -1, 0);
  if (UNLIKELY(reinterpret_cast<void*>(p) == MAP_FAILED)) {
    return nullptr;
  }
  if (hint != p) {
    munmap(p, size);
    return nullptr;
  }
#if defined(SCALLOC_DISABLE_TRANSPARENT_HUGEPAGES) && defined(MADV_NOHUGEPAGE)
  if (madvise(p, size, MADV_NOHUGEPAGE) != 0) {
    Fatal("madvise MADV_NOHUGEPAGE failed");
  }
#endif  // SCALLOC_DISABLE_TRANSPARENT_HUGEPAGES && MADV_NOHUGEPAGE
  return p;
}


always_inline void* SystemMmap(size_t size) {
  const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  const int prot = PROT_READ | PROT_WRITE;
  void* p = mmap(0, size, prot, flags, -1, 0);
  if (UNLIKELY(reinterpret_cast<void*>(p) == MAP_FAILED)) {
    return nullptr;
  }
#if defined(SCALLOC_DISABLE_TRANSPARENT_HUGEPAGES) && defined(MADV_NOHUGEPAGE)
  if (madvise(p, size, MADV_NOHUGEPAGE) != 0) {
    Fatal("madvise MADV_NOHUGEPAGE failed");
  }
#endif  // SCALLOC_DISABLE_TRANSPARENT_HUGEPAGES && MADV_NOHUGEPAGE
  return p;
}


always_inline void* SystemMmapFail(size_t size) {
  void* p = SystemMmap(size);
  if (UNLIKELY(p == nullptr)) { Fatal("mmap failed"); }
  return p;
}


#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
static const char log_table[256] = {
  -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
  LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};


// base-2 logarithm of 32-bit integers
always_inline int Log2(size_t v) {
  unsigned int t, tt, r;  // temp vars
  if ((tt = (v >> 16))) {
    r =  (t = (tt >> 8)) ? 24 + log_table[t] : 16 + log_table[tt];
  } else {
    r =  (t = (v >> 8)) ? 8 + log_table[t] : log_table[v];
  }
  return r;
}


always_inline int32_t CpusOnline() {
  static __attribute__((aligned(64))) int32_t cpus_online = -1;
  if (cpus_online == -1) {
    cpus_online = static_cast<int32_t>(sysconf(_SC_NPROCESSORS_ONLN));
    if ((cpus_online == - 1) ||  /* sysconf failed */
        (cpus_online == 0)) {    /* this should not happen */
      Fatal("CpusOnline failed");
    }
  }
  return cpus_online;
}


always_inline uint_fast64_t rdtsc(void) {
  unsigned int hi, lo;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint_fast64_t) lo) | (((uint_fast64_t) hi) << 32);
}


always_inline uint_fast64_t hwrand() {
  return (rdtsc() >> 6);
}


// Used for pseudorand
const uint32_t kA = 16807;
const uint32_t kM = 2147483647;
const uint32_t kQ = 127773;
const uint32_t kR = 2836;

always_inline uint64_t pseudorand() {
  static int32_t s = 1237;
  int32_t seed = s;
  uint32_t hi = seed / kQ;
  uint32_t lo = seed % kQ;
  seed = kA * lo - kR * hi;
  if (seed < 0) {
    seed += kM;
  }
  s = seed;
  return seed;
}

}  // namespace scalloc

#endif  // SCALLOC_UTILS_H_

// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "utils.h"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>  // mmap
#include <unistd.h>

#include "common.h"
#include "platform/assert.h"

namespace {


//inline void Swap(size_t& a, size_t& b) {
//  if (&a == &b) {
//    return;
//  }
//  a ^= b;
//  b ^= a;
//  a ^= b;
//}

//inline size_t Gcd(size_t a, size_t b) {
//  if (b < a) {
//    Swap(a, b);
//  }
//
//  size_t t;
//  while (b != 0) {
//    t = b;
//    b = a % b;
//    a = t;
//  }
//  return a;
//}

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
  ScallocAssert(cpus != 0);
  return cpus;
}


/*
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
*/

void* SystemMmap(size_t size, size_t* actual_size, bool huge) {
  // pad size to some multiple of the system page size
  size = utils::PadSize(size, kPageSize);

  if (actual_size) {
    *actual_size = size;
  }

  const int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef HAVE_HUGEPAGE
  if (huge) {
    flags |= MAP_HUGETLB;
  }
#endif  // HAVE_HUGEPAGE
  void* p = mmap(0, size, prot, flags, -1, 0);
  if (reinterpret_cast<void*>(p) == MAP_FAILED) {
    Fatal("mmap failed. size: %lu, errno: %lu\n", size, errno);
  }
  ScallocAssert(reinterpret_cast<uintptr_t>(p) % kPageSize == 0);
  return p;
}


void SystemMunmap(void* p, const size_t actual_size) {
  if (munmap(p, actual_size) != 0) {
    if (reinterpret_cast<void*>(p) == MAP_FAILED) {
      Fatal("munmap failed. p: %p, errno: %lu\n", p, errno);
    }
  }
}


namespace {

void SkipUntil(const int fd, char until, char* buf) {
  char c;
  while (read(fd, &c, 1) == 1) {
    if (c == until) {
      return;
    }
    if (buf != NULL) {
      strncat(buf, &c, 1);
    }
  }
}

}  // namespace


// Read a proc entry without using fopen/fread and friends since they may call
// malloc themselves.
int64_t ReadProcEntry(const int fd, const char* entry_name) {
  const size_t entry_len = strlen(entry_name);
  char line[256] = { 0 };
  size_t char_cnt = 0;
  char c;

  while (read(fd, &c, 1) == 1) {
    line[char_cnt] = c;
    char_cnt++;
    if (char_cnt == entry_len) {
      memset(&line, 0, char_cnt);  // Reset line buffer.
      if (strcmp(line, entry_name) == 0) {
        // Found entry.
        if (read(fd, &c, 1) != 1) {  // Read away ':'.
          Fatal("unexpected eof");
        }
        SkipUntil(fd, '\n', line);
        return strtoll(line, NULL, 10);
      } else {
        // Skip till newline.
        SkipUntil(fd, '\n', NULL);
        char_cnt = 0;
      }
    }
  }
  return -1;
}

}  // namespace utils
}  // namespace scalloc

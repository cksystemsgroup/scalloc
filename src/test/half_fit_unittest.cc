// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <sys/mman.h>  // mmap

#include "allocators/half_fit.h"

namespace {

void* GetMem(size_t size) {
  void* p = mmap(0,
                 size,
                 PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
                 -1,
                 0);
  if (p == MAP_FAILED) {
    perror("mmap");
    abort();
  }
  return p;
}

}  // namespace

TEST(HalfFit, Init) {
  size_t size = 1UL << 28;
  void* p = GetMem(size);
  HalfFit* hf = reinterpret_cast<HalfFit*>(p);
  hf->Init(p, size);
}

TEST(HalfFit, Alloc) {
  size_t size = 1UL << 28;
  void* p = GetMem(size);
  HalfFit* hf = reinterpret_cast<HalfFit*>(p);
  hf->Init(p, size);
  hf->Allocate(1UL << 10);
  ASSERT_FALSE(hf->Empty());
}

TEST(HalfFit, AllocFree) {
  size_t size = 1UL << 28;
  void* p = GetMem(size);
  HalfFit* hf = reinterpret_cast<HalfFit*>(p);
  hf->Init(p, size);
  void* np =hf->Allocate(1UL << 10);
  hf->Free(np);
  ASSERT_TRUE(hf->Empty());
}

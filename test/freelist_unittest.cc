// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <algorithm>
#include <vector>

#include "freelist-inl.h"
#include "size_classes.h"


void test_size(scalloc::FreelistBase* l) {
  using namespace scalloc;
  std::vector<void*> used;
  const size_t size = 16;
  const size_t size_class = SizeToClass(size);
  void* mem = malloc(ClassToSpanSize[size_class]);

  l->Init(mem, size_class);
  void* obj;
  while ((obj = l->Pop()) != NULL) {
    used.push_back(obj);
  }
  EXPECT_EQ(l->Size(), 0);
  l->Push(used[0]);
  EXPECT_EQ(l->Size(), 1);
  EXPECT_EQ(used[0], l->Pop());
  EXPECT_EQ(l->Size(), 0);
  for (std::vector<void*>::iterator obj_it = used.begin(); obj_it != used.end(); ++obj_it) {
    l->Push(*obj_it);
  }
  EXPECT_EQ(l->Utilization(), 0);
}


void test_locality(scalloc::FreelistBase* l) {
  using namespace scalloc;
  const size_t size = 16;
  const size_t size_class = SizeToClass(size);
  void* mem = malloc(ClassToSpanSize[size_class]);
  l->Init(mem, size_class);
  void* pop1 = l->Pop();
  l->Push(pop1);
  void* pop2 = l->Pop();
  EXPECT_EQ(pop1, pop2);
  free(mem);
}


TEST(BatchedFreelist, Size) {
  scalloc::BatchedFreelist l;
  test_size(&l);
}


TEST(IncrementalFreelist, Size) {
  scalloc::IncrementalFreelist l;
  test_size(&l);
}


TEST(BatchedFreelist, Locality) {
  scalloc::BatchedFreelist l;
  test_locality(&l);
}


TEST(IncrementalFreelist, Locality) {
  scalloc::IncrementalFreelist l;
  test_locality(&l);
}

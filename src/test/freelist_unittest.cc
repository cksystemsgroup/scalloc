// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <algorithm>
#include <vector>

#include "freelist.h"

namespace {

// sizes to test
const size_t kSizes[] = {8, 16, 17, 23, 48, 73, 128, 256,512};

// block sizes to test
const size_t kBlockSizes[] = {1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14};

}

TEST(Freelist, Size) {
  std::vector<void*> used;  
  Freelist l;
  const size_t size = 16;
  const size_t block_size = 1024;
  void* mem = malloc(block_size);
  l.FromBlock(mem, size, block_size/size);
  void* obj;
  while((obj = l.Pop()) != NULL) {
    used.push_back(obj);
  }
  EXPECT_EQ(l.Size(), 0);
  l.Push(used[0]);
  EXPECT_EQ(l.Size(), 1);
  EXPECT_EQ(used[0], l.Pop());
  EXPECT_EQ(l.Size(), 0);
  for (std::vector<void*>::iterator obj_it = used.begin(); obj_it != used.end(); ++obj_it) {
    l.Push(*obj_it);
  }
  EXPECT_EQ(l.Size(), block_size/size);
  free(mem);
}

TEST(Freelist, Locality) {
  Freelist l;
  const size_t size = 16;
  const size_t block_size = 1024;
  void* mem = malloc(block_size);
  l.FromBlock(mem, size, block_size/size);
  void* pop1 = l.Pop();
  l.Push(pop1);
  void* pop2 = l.Pop();
  EXPECT_EQ(pop1, pop2);
  free(mem);
}

// check whether Freelist::FromBlock correctly initializes the freelist
TEST(Freelist, FromBlockIntegrity) {
  std::vector<size_t> sizes(
      kSizes, kSizes + sizeof(kSizes) / sizeof(size_t));
  std::vector<size_t> block_sizes(
      kBlockSizes, kBlockSizes + sizeof(kBlockSizes) / sizeof(size_t));
  // temporary stores already Pop()'ed objects to check whether the freelist
  // contains duplicates
  std::vector<uintptr_t> used;  
  Freelist l;

  for (std::vector<size_t>::iterator block_it = block_sizes.begin(); block_it != block_sizes.end(); ++block_it) {
    size_t block_size = *block_it;
    for (std::vector<size_t>::iterator size_it = sizes.begin(); size_it != sizes.end(); ++size_it) {
      used.clear();
      size_t size = *size_it;
      void* mem = malloc(block_size);
      uintptr_t mem_ptr = reinterpret_cast<uintptr_t>(mem);
      l.FromBlock(mem, size, block_size/size);
      void* obj;
      while((obj = l.Pop()) != NULL) {
        uintptr_t obj_ptr = reinterpret_cast<uintptr_t>(obj);
        EXPECT_GE(obj_ptr, mem_ptr);
        EXPECT_LT(obj_ptr, mem_ptr + block_size);
        ASSERT_FALSE(std::find(used.begin(), used.end(), obj_ptr) != used.end());
        used.push_back(obj_ptr);
      }
      EXPECT_EQ(used.size(), block_size/size);
      free(mem);
    }
  }

}

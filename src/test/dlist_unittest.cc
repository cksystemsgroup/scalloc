// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include "arena.h"
#include "dlist.h"
#include "runtime_vars.h"

namespace {

void Init() {
  RuntimeVars::InitModule();
  InitArenas();
}

}

TEST(DList, Init) {
  Init();
  DList<uint64_t> list;
  list.Init();
}

TEST(DList, InsertRemove) {
  Init();
  DList<uint64_t> list;
  list.Init();

  list.Insert(1);
  ASSERT_FALSE(list.Empty());
  list.Remove(1);
  ASSERT_TRUE(list.Empty());
}

TEST(DList, Iterator) {
  Init();
  DList<uint64_t> list;
  list.Init();

  list.Insert(1);
  list.Insert(2);
  for (DList<uint64_t>::iterator it = list.begin(); it != list.end(); it = it->next) {
    ASSERT_TRUE(it->data == 1 || it->data == 2);    
  }

  list.Remove(2);
  for (DList<uint64_t>::iterator it = list.begin(); it != list.end(); it = it->next) {
    ASSERT_TRUE(it->data == 1);
  }

  list.Remove(1);
  ASSERT_TRUE(list.Empty());
}


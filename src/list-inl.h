
// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LIST_INL_H_
#define SCALLOC_LIST_INL_H_

#include <stdint.h>

class ListNode {
 public:
  ListNode* next;
  ListNode* prev;
  void* data;
};

#endif  // SCALLOC_LIST_INL_H_


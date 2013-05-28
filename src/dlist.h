// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_DLIST_H_
#define SCALLOC_DLIST_H_

#include "common.h"
#include "page_heap_allocator.h"
#include "runtime_vars.h"

template<typename T>
class DList {
 public:
  template<typename S>
  struct Node {
    Node* next;
    Node* prev;
    S data;
  };

  typedef Node<T>* iterator;
  typedef const Node<T>* const_iterator;

  iterator begin() { return list_; }
  iterator end() { return NULL; }

  void Init();
  void Insert(T data);
  void Remove(T data);
  bool Empty();
  size_t Len();

 private:
  size_t len_;
  Node<T>* list_;
  cache_aligned scalloc::PageHeapAllocator<Node<T>> allocator_;

  void RemoveNode(Node<T>* n);
} cache_aligned;

template<typename T>
inline void DList<T>::Init() {
  allocator_.Init(RuntimeVars::SystemPageSize());
  list_ = NULL;
  len_ = 0;
}

template<typename T>
inline bool DList<T>::Empty() {
  return len_ == 0;
}

template<typename T>
inline size_t DList<T>::Len() {
  return len_;
}

template<typename T>
inline void DList<T>::Insert(T data) {
  Node<T>* n = allocator_.New();
  n->next = list_;
  n->prev = NULL;
  n->data = data;
  if (list_ != NULL) {
    list_->prev = n;
  }
  list_ = n;
}

template<typename T>
inline void DList<T>::Remove(T data) {
  Node<T>* n = list_;
  while (n != NULL) {
    if (n->data == data) {
      RemoveNode(n);
      Node<T>* next = n->next;
      allocator_.Delete(n);
      n = next;
    } else {
      n = n->next;
    }
  }
}

template<typename T>
inline void DList<T>::RemoveNode(Node<T>* n) {
  if (n->prev) {
    n->prev->next = n->next;
  }
  if (n->next) {
    n->next->prev = n->prev;
  }
  if (list_ == n) {
    list_ = n->next;
  }
}

#endif  // SCALLOC_DLIST_H_

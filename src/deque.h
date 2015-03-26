// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_DEQUE_H_
#define SCALLOC_DEQUE_H_

#include "core_id.h"
#include "globals.h"
#include "lock.h"
#include "log.h"

namespace scalloc {

class DoubleListNode {
 public:
  always_inline DoubleListNode() : next_(nullptr), prev_(nullptr) { }
  always_inline DoubleListNode* next() { return next_; }
  always_inline DoubleListNode* set_next(DoubleListNode* next) {
    ScallocAssert(next != nullptr);
    return next_ = next;
  }
  always_inline DoubleListNode* prev() { return prev_; }
  always_inline DoubleListNode* set_prev(DoubleListNode* prev) {
    ScallocAssert(prev != nullptr);
    return prev_ = prev;
  }
  always_inline void clear_prev() { prev_ = nullptr; }
  always_inline void clear_next() { next_ = nullptr; }

 private:
  DoubleListNode* next_;
  DoubleListNode* prev_;
};


// A simple sequential double-ended queue (deque) allowing constant time insert
// (front, back) and remove (front, back, and specific node).
class Deque {
 public:
  always_inline Deque();
  always_inline void PushFront(core_id owner, DoubleListNode* node);
  always_inline void PushBack(core_id owner, DoubleListNode* node);
  always_inline void Remove(core_id owner, DoubleListNode* node);

  always_inline DoubleListNode* RemoveFront();
  always_inline DoubleListNode* RemoveBack();
  always_inline void RemoveAll();

  always_inline void Open(core_id owner);
  always_inline void Close();

 private:
  typedef SpinLock<0> Lock;

  always_inline DoubleListNode* sentinel() { return &sentinel_; }

  Lock lock_;
  core_id owner_;
  DoubleListNode sentinel_;

  UNUSED char pad_[64 - ((
      sizeof(lock_) +
      sizeof(sentinel_)) % 64)];
};


void Deque::Open(core_id owner) {
  owner_ = owner;
}


void Deque::Close() {
  owner_ = kTerminated;
}


Deque::Deque() {
  lock_.Unlock();
  sentinel()->set_next(sentinel());
  sentinel()->set_prev(sentinel());
}


void Deque::PushFront(core_id owner, DoubleListNode* node) {
  Lock::Guard guard(lock_);
  ScallocAssert(node != nullptr);
  if (owner != owner_) { return; }

  node->set_prev(sentinel());
  node->set_next(sentinel()->next());
  sentinel()->next()->set_prev(node);
  sentinel()->set_next(node);
}


void Deque::PushBack(core_id owner, DoubleListNode* node) {
  Lock::Guard guard(lock_);
  ScallocAssert(node != nullptr);
  if (owner != owner_) { return; }

  node->set_prev(sentinel()->prev());
  node->set_next(sentinel());
  sentinel()->prev()->set_next(node);
  sentinel()->set_prev(node);
}


DoubleListNode* Deque::RemoveFront() {
  Lock::Guard guard(lock_);

  DoubleListNode* node = sentinel()->next();
  sentinel()->set_next(node->next());
  node->next()->set_prev(sentinel());
  if (node == sentinel()) { return nullptr; }
  node->clear_prev();
  node->clear_next();
  return node;
}


DoubleListNode* Deque::RemoveBack() {
  Lock::Guard guard(lock_);

  DoubleListNode* node = sentinel()->prev();
  sentinel()->set_prev(node->prev());
  node->prev()->set_next(sentinel());
  if (node == sentinel()) { return nullptr; }
  node->clear_prev();
  node->clear_next();
  return node;
}


void Deque::Remove(core_id owner, DoubleListNode* node) {
  Lock::Guard guard(lock_);
  if ((node->prev() == nullptr) && (node->next() == nullptr)) { return; }
  if (owner != owner_) { return; }

  ScallocAssert(node->next() != nullptr && node->prev() != nullptr);

  if (node->next()) { node->next()->set_prev(node->prev()); }
  if (node->prev()) { node->prev()->set_next(node->next()); }
  node->clear_prev();
  node->clear_next();
}


void Deque::RemoveAll() {
  // No lock here! ;)
  DoubleListNode* node;
  while ((node = RemoveFront()) != nullptr) { }
}

}   // namespace scalloc

#endif  // SCALLOC_DEQUE_H_


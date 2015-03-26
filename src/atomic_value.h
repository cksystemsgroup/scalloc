// Copyright (c) 2014, the Scal Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCAL_UTIL_ATOMIC_VALUE_NEW_H_
#define SCAL_UTIL_ATOMIC_VALUE_NEW_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <atomic>
#include <limits>

#include "platform/globals.h"

#ifdef DEBUG
#define TAGGED_VALUE_CHECKED_MODE 1
#endif  // DEBUG

template<typename T, int ALIGN, int PAD> class AtomicTaggedValue;

template<typename T>
class TaggedValue {
 public:
  typedef uint64_t raw_type;
  typedef uint16_t tag_type;
  typedef T value_type;

  static const tag_type kMaxTag = (1UL << 16) - 1;

  static always_inline void CheckCompatibility(T value) {
#ifdef TAGGED_VALUE_CHECKED_MODE
    assert((value & kValueMask) == value);
#endif  // TAGGED_VALUE_CHECKED_MODE
  }

  always_inline TaggedValue() : raw_(0) {}

  explicit always_inline TaggedValue(T value, tag_type tag)
      : raw_((reinterpret_cast<raw_type>(value) & kValueMask) |
             (static_cast<raw_type>(tag) << 48)) {
#ifdef TAGGED_VALUE_CHECKED_MODE
    if ((this->value() != value) || (this->tag() != tag)) {
      fprintf(stderr, "TaggedValue inconsistency: tried creating with "
                      "value: %p, tag: %d. result: value(): %p, tag(): %d.\n",
                      (void*)value, tag, (void*)this->value(), this->tag());  // NOLINT
      abort();
    }
#endif  // TAGGED_VALUE_CHECKED_MODE
    static_assert(sizeof(T) <= 8, /* only approximate check */
                 "tagging requires the value to have less than 48 bits, "
                 "or 48 bits with the 48'th bit being extended");
  }

  always_inline TaggedValue(const TaggedValue& other) {
    // See operator=.
    (*this) = other;
  }

  always_inline T value() const {
    return reinterpret_cast<T>(
        (raw_ & kValueMask) |
        (((raw_ >> (kValueBits - 1)) & 0x1) * kExtendMask));
  }

  always_inline tag_type tag() const {
    return static_cast<tag_type>(raw_ >> kValueBits);
  }

  always_inline bool operator==(const TaggedValue<T>& other) const {
    return raw_ == other.raw_;
  }

  always_inline bool operator!=(const TaggedValue<T>& other) const {
    return !(*this == other);
  }

  // No copy-and-swap since we only need to manage primitive members.
  always_inline TaggedValue<T>& operator=(const TaggedValue<T>& other) {
    raw_ = other.raw_;
    return *this;
  }

 private:
  static const uint64_t kValueBits = 48;
  static const uint64_t kValueMask = (1UL << kValueBits) - 1;
  static const uint64_t kExtendMask =
      std::numeric_limits<raw_type>::max() - kValueMask;

  explicit always_inline TaggedValue(raw_type raw) : raw_(raw) {}

  raw_type raw_;

  template<typename S, int ALIGN, int PAD>
  friend class AtomicTaggedValue;
};


template<typename T, int ALIGN = 0, int PAD = 0>
class AtomicTaggedValue {
 public:
  always_inline AtomicTaggedValue() : raw_atomic_(0) {}

  explicit always_inline AtomicTaggedValue(const TaggedValue<T>& tagged_value)
      : raw_atomic_(tagged_value.raw_) {}

  always_inline TaggedValue<T> load() const {
    return TaggedValue<T>(raw_atomic_.load());
  }

  always_inline void store(const TaggedValue<T>& tagged_value) {
    raw_atomic_.store(tagged_value.raw_);
  }

  always_inline bool swap(const TaggedValue<T>& expected,  // NOLINT
                          const TaggedValue<T>& desired) {
    uint64_t val = expected.raw_;
    return raw_atomic_.compare_exchange_strong(val, desired.raw_);
  }

  always_inline void* operator new(size_t size) {
    if (ALIGN == 0) {
      return malloc(size);
    }
    void* mem;
    if (posix_memalign(&mem, ALIGN, size) != 0) {
      fprintf(stderr, "posix_memalign of AtomicTaggedValue failed\n");
      abort();
    }
    return mem;
  }

  always_inline void operator delete(void* ptr) {
    free(ptr);
  }

 private:
  std::atomic<typename TaggedValue<T>::raw_type> raw_atomic_;
  uint8_t _padding[ (PAD != 0) ?
      PAD - sizeof(std::atomic<typename TaggedValue<T>::raw_type>) : 0 ];
};

#endif  // SCAL_UTIL_ATOMIC_VALUE_NEW_H_

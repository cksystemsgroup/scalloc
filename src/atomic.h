// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ATOMIC_H_
#define SCALLOC_ATOMIC_H_

#include <stdint.h>

#include "assert.h"

// Packed structure for ABA stamped pointers.
// Not concurrency safe. Use with std::atomic.
template<typename ValueType, typename TagType, int TagWidth = 4>
class TaggedValue {
 public:
  typedef uint64_t RawType;

  inline TaggedValue() : raw(0) {}
  inline TaggedValue(ValueType atomic, TagType tag) {
    Pack(atomic, tag);
  }

  void Pack(ValueType atomic, TagType tag);
  ValueType Value() const;
  TagType Tag() const;

  RawType raw;

 private:
  static const uint8_t kWidth = sizeof(raw) * 8;
  static const uint8_t kTagWidth = TagWidth;
  static const uint64_t kValueMask = (1ULL << (kWidth - kTagWidth)) - 1;
  static const uint64_t kTagMask = ~kValueMask;

  DISALLOW_ALLOCATION();
  DISALLOW_COPY_AND_ASSIGN(TaggedValue);
};


template<typename ValueType, typename TagType, int TagWidth>
inline void TaggedValue<ValueType, TagType, TagWidth>::Pack(ValueType atomic,
                                                            TagType tag) {
  raw = 0;
  raw |= (((RawType)(atomic) >> kTagWidth) & kValueMask);
  raw |= (((RawType)(tag) << (kWidth - kTagWidth)) & kTagMask);
}


template<typename ValueType, typename TagType, int TagWidth>
inline ValueType TaggedValue<ValueType, TagType, TagWidth>::Value() const {
  return (ValueType)((raw & kValueMask) << kTagWidth);
}


template<typename ValueType, typename TagType, int TagWidth>
inline TagType TaggedValue<ValueType, TagType, TagWidth>::Tag() const {
  return (TagType)((raw & kTagMask) >> (kWidth - kTagWidth));
}


typedef unsigned int uint128_t __attribute__((mode(TI)));
#define UINT128_C(X) (0u + ((uint128_t)(X)))


template<typename ValueType>
class TaggedValue128 {
 public:
  inline TaggedValue128() : raw(0) {}
  inline TaggedValue128(ValueType atomic, uint64_t tag) {
    Pack(atomic, tag);
  }

  void Pack(ValueType atomic, uint64_t tag);
  ValueType Value() const;
  uint64_t Tag() const;

  uint128_t raw;
 private:
};


template<typename ValueType>
inline void TaggedValue128<ValueType>::Pack(ValueType atomic, uint64_t tag) {
  // This is a weak pack. Only use in thread-local scenarios.
  raw = 0;
  raw |= ((uint128_t)tag);
  raw |= ((uint128_t)atomic) << 64;
}



template<typename ValueType>
inline ValueType TaggedValue128<ValueType>::Value() const {
  return (ValueType)(raw >> 64);
}


template<typename ValueType>
inline uint64_t TaggedValue128<ValueType>::Tag() const {
  return (uint64_t)(raw);
}

#endif  // SCALLOC_ATOMIC_H_

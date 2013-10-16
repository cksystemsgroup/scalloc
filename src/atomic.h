// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ATOMIC_H_
#define SCALLOC_ATOMIC_H_

#include <stdint.h>

#include "assert.h"

// Packed structure for ABA stamped pointers.
// Not concurrency safe. Use with std::atomic.
template<typename ValueType, typename TagType, int TagWidth = 4> class TaggedValue {
 public:
  inline TaggedValue() : raw(0) {}
  inline TaggedValue(ValueType atomic, TagType tag) {
    Pack(atomic, tag);
  }

  void Pack(ValueType atomic, TagType tag);
  ValueType Value() const;
  TagType Tag() const;

  typedef uint64_t RawType;

  RawType raw;

 private:
  static const uint8_t kWidth = sizeof(raw) * 8;
  static const uint8_t kTagWidth = TagWidth;
  static const uint64_t kValueMask = (1ULL << (kWidth - kTagWidth)) - 1;
  static const uint64_t kTagMask = ~kValueMask;

  DISALLOW_COPY_AND_ASSIGN(TaggedValue);
  DISALLOW_ALLOCATION();
};

template<typename ValueType, typename TagType, int TagWidth>
inline void TaggedValue<ValueType, TagType, TagWidth>::Pack(ValueType atomic, TagType tag) {
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

#endif  // SCALLOC_ATOMIC_H_

// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ATOMIC_H_
#define SCALLOC_ATOMIC_H_

#include <stdint.h>

// Packed structure for tagged values (ABA stamped pointers). The structure
// is *NOT* concurrency safe. Use within std::atomic!
// Layout: |---tag---|---value---|
template<typename ValueType, typename TagType, int TagWidth = 4>
class TaggedValue {
 public:
  inline TaggedValue(ValueType value, TagType tag);
  void Pack(ValueType value, TagType tag);
  inline ValueType Value() const;
  inline TagType Tag() const;

  uint64_t raw;

 private:
  static const int kWidth = sizeof(uint64_t);
  static const int kTagWidth = TagWidth;
  static const uint64_t kValueMask = (1ULL << (kWidth - TagWidth)) - 1;
  static const uint64_t kTagMask = ~kValueMask;
};

template<typename ValueType, typename TagType, int TagWidth>
TaggedValue<ValueType, TagType, TagWidth>::TaggedValue(
    ValueType value, TagType tag) {
  Pack(value, tag);
}

template<typename ValueType, typename TagType, int TagWidth>
void TaggedValue<ValueType, TagType, TagWidth>::Pack(
    ValueType value, TagType tag) {
  raw = 0;
  raw |= (((uint64_t)(value) >> kTagWidth) & kValueMask);
  raw |= (((uint64_t)(tag) << (kWidth - kTagWidth)) & kTagMask);
}

template<typename ValueType, typename TagType, int TagWidth>
ValueType TaggedValue<ValueType, TagType, TagWidth>::Value() const {
  return (ValueType)((raw & kValueMask) << kTagWidth);
}

template<typename ValueType, typename TagType, int TagWidth>
TagType TaggedValue<ValueType, TagType, TagWidth>::Tag() const {
  return (TagType)(raw & kTagMask);
}

#endif  // SCALLOC_ATOMIC_H_

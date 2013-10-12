// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_ATOMIC_H_
#define SCALLOC_ATOMIC_H_

#include <stdint.h>

typedef uint64_t Atomic64;

#if defined(__i386__)  // IA-32

inline Atomic64 Atomic64_Read(const Atomic64* other) {
  return __sync_val_compare_and_swap(other, 0, 0);
}

inline void Atomic64_Write(Atomic64* var, Atomic64 value) {
  Atomic64 old;
  do {
    old = Atomic64_Read(var);
  } while (!__sync_bool_compare_and_swap(var, old, value));
}

#elif defined(__x86_64__)  // AMD64

inline Atomic64 Atomic64_Read(const Atomic64* var) {
  return *const_cast<const volatile Atomic64*>(var);
}

inline void Atomic64_Write(Atomic64* var, Atomic64 value) {
  *const_cast<volatile Atomic64*>(var) = value;
}

#else
# error platform not supported
#endif

// Packed structure for ABA stamped pointers.
//
// Memory layout:
// |---tag---|---atomic---|
//
// We rely on the fact that RawType (uint64_t) can be read in a single CPU
// instruction. This is the case for __x86_64__.
template<typename AtomicType,
         typename TagType,
         int AtomicWidth = 64,
         int AtomicAlign = 4>
class TaggedAtomic {
 public:
  inline TaggedAtomic() : raw_(0) {}

  explicit inline TaggedAtomic(AtomicType atomic) {
    Pack(atomic, 0);
  }

  inline TaggedAtomic(AtomicType atomic, TagType tag) {
    Pack(atomic, tag);
  }

  inline TaggedAtomic(const TaggedAtomic& other) {
    // no need for atomic write in (copy) ctor
    raw_ = Atomic64_Read(&(other.raw_));
  }

  // We do not like overloading operators...
  inline void CopyFrom(const TaggedAtomic& other) {
    if (this != &other) {  // no self copies
      Atomic64_Write(&raw_, Atomic64_Read(&(other.raw_)));
    }
  }

  int GetAtomicWidth() const {
    return AtomicWidth;
  }

  int GetAtomicAlign() const {
    return AtomicAlign;
  }

  void Pack(AtomicType atomic, TagType tag);
  AtomicType Atomic() const;
  TagType Tag() const;
  bool AtomicExchange(const TaggedAtomic& expected, const TaggedAtomic& neww);

  void WeakPack(AtomicType atomic, TagType tag);

 private:
  typedef uint64_t RawType;

  static const uint64_t AtomicMask = (1ULL << (AtomicWidth - AtomicAlign)) - 1;
  static const uint64_t TagMask = ~AtomicMask;

  RawType raw_;
};

template<typename AtomicType,
         typename TagType,
         int AtomicWidth,
         int AtomicAlign>
inline void TaggedAtomic<AtomicType, TagType, AtomicWidth, AtomicAlign>::Pack(
    AtomicType atomic, TagType tag) {
  Atomic64 tmp = 0;
  // Ignores the cast iff AtomicType == uintptr_t, otherwise it will do a
  // reinterpret_cast.
  tmp |= (((uintptr_t)(atomic) >> AtomicAlign) & AtomicMask);
  tmp |= ((tag << (AtomicWidth - AtomicAlign)) & TagMask);
  Atomic64_Write(&raw_, tmp);
}

template<typename AtomicType,
         typename TagType,
         int AtomicWidth,
         int AtomicAlign>
inline void TaggedAtomic<AtomicType, TagType, AtomicWidth, AtomicAlign>::WeakPack(AtomicType atomic, TagType tag) {
  // Ignores the cast iff AtomicType == uintptr_t, otherwise it will do a
  // reinterpret_cast.
  raw_ = 0;
  raw_ |= (((uintptr_t)(atomic) >> AtomicAlign) & AtomicMask);
  raw_ |= ((tag << (AtomicWidth - AtomicAlign)) & TagMask);
}

template<typename AtomicType, typename TagType, int AtomicWidth, int AtomicAlign>
inline AtomicType TaggedAtomic<AtomicType, TagType, AtomicWidth, AtomicAlign>::Atomic() const {
  Atomic64 cpy = Atomic64_Read(&raw_);
  RawType a = (cpy & AtomicMask) << AtomicAlign;
  return (AtomicType)a;
}

template<typename AtomicType, typename TagType, int AtomicWidth, int AtomicAlign>
inline TagType TaggedAtomic<AtomicType, TagType, AtomicWidth, AtomicAlign>::Tag() const {
  Atomic64 cpy = Atomic64_Read(&raw_);
  RawType t = (cpy & TagMask) >> (AtomicWidth - AtomicAlign);
  return (TagType)t;
}

template<typename AtomicType, typename TagType, int AtomicWidth, int AtomicAlign>
inline bool TaggedAtomic<AtomicType, TagType, AtomicWidth, AtomicAlign>::AtomicExchange(const TaggedAtomic& expected, const TaggedAtomic& neww) {
  if ((Atomic64_Read(&raw_) == expected.raw_) &&
      __sync_bool_compare_and_swap(&raw_, expected.raw_, neww.raw_)) {
    return true;
  }
  return false;
}

#endif  // SCALLOC_ATOMIC_H_

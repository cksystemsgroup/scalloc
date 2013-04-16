#ifndef SCALLOC_COMMON_H_
#define SCALLOC_COMMON_H_

#define UNLIKELY(x)   __builtin_expect((x), 0)
#define LIKELY(x)     __builtin_expect((x), 1)

#define cache_aligned __attribute__((aligned(64)))

#define always_inline inline __attribute__((always_inline))
#define no_inline __attribute__((noinline))

#endif  // SCALLOC_COMMON_H_

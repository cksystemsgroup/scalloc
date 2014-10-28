#ifndef SCALLOC_BUFFER_LAB_H_
#define SCALLOC_BUFFER_LAB_H_

#include <pthread.h>
#include <stdint.h>

#include "common.h"
#include "log.h"

#define MAX_LABS 256

namespace scalloc {

class ScallocCore;
class RRAllocationBuffer;

class AllocationBuffer {
 public:
  always_inline void SetAllocator(ScallocCore* allocator) { allocator_ = allocator; }
  always_inline ScallocCore* Allocator() { return allocator_; }
  always_inline void Acquire() { if (threads == 0) used_ = 1; }
  always_inline void Release() { if (threads == 0) used_ = 0; }

  void MakeLocked();
  void ResetAllocator();

  inline uint64_t NextSpanRefill(uint64_t sc);

  uint_fast64_t threads;
  volatile int_fast8_t used_;
  uint64_t span_refill_[kNumClasses];

 private:
  ScallocCore* allocator_;
};

class RRAllocationBuffer {
 public:
  static void ThreadDestructor(void* lab);

  void Init();
  inline bool Enabled() { return enabled_; }
  inline AllocationBuffer& GetAllocationBuffer(void* object);


 private:
  pthread_key_t lab_key_;
  uint64_t thread_counter_;
  uint64_t num_labs_;
  AllocationBuffer* labs_[MAX_LABS];
  bool enabled_;  // Linker initialized with 0.
};

inline AllocationBuffer& RRAllocationBuffer::GetAllocationBuffer(void* object) {
  AllocationBuffer* lab = reinterpret_cast<AllocationBuffer*>(
      pthread_getspecific(lab_key_));
  if (lab == NULL) {
    // Assign to a LAB in RR fashion.
    uint64_t lab_id = __sync_fetch_and_add(&thread_counter_, 1) % num_labs_;
    lab = labs_[lab_id];
    __sync_fetch_and_add(&lab->threads, 1);
    if (lab->threads > 1) {
      lab->MakeLocked();
      MemoryBarrier();
      while (lab->used_ != 0);
    }
    pthread_setspecific(lab_key_, (void*)lab);
  }
  lab->Acquire();
  return *lab;
}


inline uint64_t AllocationBuffer::NextSpanRefill(uint64_t sc) {
  uint64_t ret = span_refill_[sc];
  span_refill_[sc] *= 2;
  return ret;
}

}  // namespace scalloc

#endif  // SCALLOC_BUFFER_LAB_H_

// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PROFILER_H_
#define SCALLOC_PROFILER_H_

#include <pthread.h>
#include <stddef.h>  // size_t
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "log.h"
#include "random.h"
#include "size_map.h"

#define SCALLOC_PROFILER_METHOD_GUARD \
    if (UNLIKELY(!enabled_)) return; \
    if (UNLIKELY(self_allocating_)) return; \
    if (UNLIKELY(fp_ == NULL)) Init();

namespace scalloc {

class Profiler {
 public:
  static void Enable() {
    enabled_ = true;
  }

  static Profiler& GetProfiler();

  ~Profiler() {
    if (UNLIKELY(!enabled_)) {
      return;
    }
    if (fp_) {
      fclose(fp_);
    }
  }

  int GetType() {
    return 1;
  }

  inline void LogAllocation(size_t size) {
    SCALLOC_PROFILER_METHOD_GUARD

    size_t block_size = SizeMap::SizeToBlockSize(size);
    size_t size_class = SizeMap::SizeToClass(size);
    
    allocation_count_++;
    allocated_bytes_count_ += size;
    sizeclass_histogram_[size_class]++;

    if (size_class > 0 && size_class <= kNumClasses) {
      DecreaseRealSpanFragmentation(size_class, block_size);
    }

    if (UNLIKELY(allocated_bytes_count_ >= kTimeQuantum_)) {
      allocated_bytes_count_ = 0;
      PrintStatistics();
    }
  }

  inline void LogDeallocation(size_t size_class,
                       bool hot = true,
                       bool remote = false) {
    SCALLOC_PROFILER_METHOD_GUARD

    if (remote) {
      slow_free_count_++;
    } else {
      fast_free_count_++;
      hot ? hot_free_count_++ : warm_free_count_++;
    }

    if (size_class > 0 && size_class <= kNumClasses) {
      IncreaseRealSpanFragmentation(size_class, SizeMap::SizeToBlockSize(SizeMap::Instance().ClassToSize(size_class)));
    }
  }

  inline void LogBlockStealing() {
    SCALLOC_PROFILER_METHOD_GUARD
    block_stealing_count_++;
  }

  inline void LogSizeclassRefill() {
    SCALLOC_PROFILER_METHOD_GUARD
    sizeclass_refill_count_++;
  }

  inline void LogSpanReuse(bool remote = false) {
    SCALLOC_PROFILER_METHOD_GUARD
    remote ? remote_span_reuse_count_++ : local_span_reuse_count_++;
  }

  inline void IncreaseRealSpanFragmentation(size_t size_class, size_t size) {
    real_span_fragmentation_histogram_[size_class]+=size;
  }
  inline void DecreaseRealSpanFragmentation(size_t size_class, size_t size) {
    real_span_fragmentation_histogram_[size_class]-=size;
  }

 private:
  static const size_t kTimeQuantum_ = 1UL << 20;
  static const size_t kLargeObject = (1UL << 9) + 1;
  static bool enabled_;

  FILE *fp_;
  pthread_t tid_;
  bool self_allocating_;  // to avoid loops while Init() calls a malloc
  uint64_t sizeclass_histogram_[kNumClasses+1];
  long real_span_fragmentation_histogram_[kNumClasses+1];
  uint64_t block_stealing_count_;
  uint64_t sizeclass_refill_count_;
  uint64_t allocation_count_;
  uint64_t allocated_bytes_count_;
  uint64_t hot_free_count_;
  uint64_t warm_free_count_;
  uint64_t fast_free_count_;
  uint64_t slow_free_count_;
  uint64_t local_span_reuse_count_;
  uint64_t remote_span_reuse_count_;

  uint64_t last_medium_object_timestamp_;
  uint64_t sum_of_interarrival_times_;
  uint64_t medium_object_allocation_count_;


  void Init() {
    if (!enabled_) return;
    self_allocating_ = true;
    tid_ = pthread_self();
    char filename[128];
    snprintf(filename, sizeof(filename), "memtrace-%lu", tid_);
    fp_ = fopen(filename, "w");
    if (fp_ == NULL) {
      ErrorOut("unable to open file %s", &filename);
    }
    self_allocating_ = false;
  }

  void PrintStatistics() {
    SCALLOC_PROFILER_METHOD_GUARD

    uint64_t free_count = fast_free_count_ + slow_free_count_;
    fprintf(fp_, "Thread %lu; A %lu; R %lu; "
            "BS %lu; F %lu; H %lu; "
            "W %lu; S %lu; "
            "R/A %3.1f%%; "
            "BS/A %3.1f%%; "
            "H/FF %3.1f%%; "
            "W/FF %3.1f%%; "
            "S/F %3.1f%%; "
            "LSR/R %3.2f; "
            "RSR/R %3.2f; "
            "Medium/A %3.3f%%; "
            "tMedium(kC) %3.1f; \n",
            tid_, allocation_count_, sizeclass_refill_count_,
            block_stealing_count_, free_count, hot_free_count_,
            warm_free_count_, slow_free_count_,
            100.0*static_cast<double>(sizeclass_refill_count_) /
                static_cast<double>(allocation_count_),
            100.0*static_cast<double>(block_stealing_count_) /
                static_cast<double>(allocation_count_),
            100.0*static_cast<double>(hot_free_count_) /
                static_cast<double>(fast_free_count_),
            100.0*static_cast<double>(warm_free_count_) /
                static_cast<double>(fast_free_count_),
            100.0*static_cast<double>(slow_free_count_) /
                static_cast<double>(free_count),
            static_cast<double>(local_span_reuse_count_) /
                static_cast<double>(sizeclass_refill_count_),
            static_cast<double>(remote_span_reuse_count_) /
                static_cast<double>(sizeclass_refill_count_),
            100.0 * static_cast<double>(medium_object_allocation_count_) /
                static_cast<double>(allocation_count_),
            (static_cast<double>(sum_of_interarrival_times_) /
                static_cast<double>(medium_object_allocation_count_)) / 1000);
    fprintf(fp_, "SC-HISTO (and Frag): ");
    for (unsigned i = 1; i < kNumClasses + 1; ++i) {
      if (sizeclass_histogram_[i] > 1000)
        fprintf(fp_, "%d=%luk (", i, sizeclass_histogram_[i]/1000);
      else
        fprintf(fp_, "%d=%lu (", i, sizeclass_histogram_[i]);
      
      if (real_span_fragmentation_histogram_[i] >= (long)(1UL << 20))
        fprintf(fp_, "%ldMB);", real_span_fragmentation_histogram_[i]/(1UL << 20));
      else if (real_span_fragmentation_histogram_[i] >= (long)(1UL << 10))
        fprintf(fp_, "%ldKB);", real_span_fragmentation_histogram_[i]/(1UL << 10));
      else  
        fprintf(fp_, "%ldB);", real_span_fragmentation_histogram_[i]);
    }

    fprintf(fp_, "\n\n");
  }
} cache_aligned;

}  // namespace scalloc

#endif  // SCALLOC_PROFILER_H_

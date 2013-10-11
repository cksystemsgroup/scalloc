// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_PROFILER_H_
#define SCALLOC_PROFILER_H_

#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <pthread.h>
#include <stddef.h>  // size_t
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "random.h"
#include "size_map.h"
#include "spinlock-inl.h"

#define SCALLOC_PROFILER_METHOD_GUARD \
    if (UNLIKELY(!enabled_)) return; \
    if (UNLIKELY(self_allocating_)) return; \
    if (UNLIKELY(fp_ == NULL)) Init();

namespace scalloc {

class Profiler;  // forward declaration

class GlobalProfiler {
 public:
  static GlobalProfiler &Instance() {
    static GlobalProfiler instance;
    return instance;
  }

  void Init() {
    const char *filename = "global-memtrace";
    fp_ = fopen(filename, "w");
    if (fp_ == NULL) {
      ErrorOut("unable to open file global-memtrace");
    }
  }

  void UpdateGlobalState(uint64_t *sizeclass_histogram,
                         int64_t *real_span_fragmentation_histogram,
                         uint64_t block_stealing_count,
                         uint64_t sizeclass_refill_count,
                         uint64_t allocation_count,
                         uint64_t allocated_bytes_count,
                         uint64_t hot_free_count,
                         uint64_t warm_free_count,
                         uint64_t fast_free_count,
                         uint64_t slow_free_count,
                         uint64_t local_span_reuse_count,
                         uint64_t remote_span_reuse_count) {
    SpinLockHolder holder(&lock_);

    for (unsigned i = 0; i < kNumClasses+1; ++i) {
      sizeclass_histogram_[i] += sizeclass_histogram[i];
      real_span_fragmentation_histogram_[i] +=
          real_span_fragmentation_histogram[i];
    }
    block_stealing_count_ += block_stealing_count;
    sizeclass_refill_count_ += sizeclass_refill_count;
    allocation_count_ += allocation_count;
    allocated_bytes_count_ += allocated_bytes_count;
    hot_free_count_ += hot_free_count;
    warm_free_count_ += warm_free_count;
    fast_free_count_ += fast_free_count;
    slow_free_count_ += slow_free_count;
    local_span_reuse_count_ += local_span_reuse_count;
    remote_span_reuse_count_ += remote_span_reuse_count;

    update_count_++;

    if (update_count_ > 10) {
      update_count_ = 0;
      PrintStatistics();
    }
  }

  void LogSpanPoolPut(size_t sc) {
    SpinLockHolder holder(&lock_);
    spanpool_put_histogram_[sc]++;
  }
  void LogSpanPoolGet(size_t sc) {
    SpinLockHolder holder(&lock_);
    spanpool_get_histogram_[sc]++;
  }
  void LogSpanShrink(size_t sc) {
    SpinLockHolder holder(&lock_);
    spanpool_shrink_histogram_[sc]++;
  }


  static inline uint64_t PrettySize(uint64_t s, char *suffix) {
    if (s >= (1UL<<30)) {
      strncpy(suffix, "G", 3);
      return s / (1UL<<30);
    }
    if (s >= (1UL<<20)) {
      strncpy(suffix, "M", 3);
      return s / (1UL<<20);
    }
    if (s >= (1UL<<10)) {
      strncpy(suffix, "K", 3);
      return s / (1UL<<10);
    }
    strncpy(suffix, "", 3);
    return s;
  }


 private:
  FILE *fp_;
  SpinLock lock_;
  uint32_t update_count_;
  // global view of thread-local profiler data
  uint64_t sizeclass_histogram_[kNumClasses+1];
  int64_t real_span_fragmentation_histogram_[kNumClasses+1];
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

  // global-only state
  uint64_t spanpool_put_histogram_[kNumClasses+1];
  uint64_t spanpool_get_histogram_[kNumClasses+1];
  uint64_t spanpool_shrink_histogram_[kNumClasses+1];


  GlobalProfiler() {}
  explicit GlobalProfiler(GlobalProfiler const&);
  void operator=(GlobalProfiler const&);

  void PrintStatistics() {
    uint64_t free_count = fast_free_count_ + slow_free_count_;
    fprintf(fp_,
            "A %" PRIu64  "; R %" PRIu64 " "
            "BS %" PRIu64  " F %" PRIu64 " H %" PRIu64 " "
            "W %" PRIu64  " S %" PRIu64 " "
            "R/A %3.1f%%; "
            "BS/A %3.1f%%; "
            "H/FF %3.1f%%; "
            "W/FF %3.1f%%; "
            "S/F %3.1f%%; "
            "LSR/R %3.2f; "
            "RSR/R %3.2f; "
            "\n",
            allocation_count_, sizeclass_refill_count_,
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
                static_cast<double>(sizeclass_refill_count_));

    fprintf(fp_, "SC-HISTO (and Fragmentation):\n");
    char s1[3]; char s2[3]; char s3[3]; char s4[3]; char s5[3];
    char s6[3]; char s7[3];
    for (unsigned i = 1; i < kNumClasses + 1; ++i) {
      uint64_t blocksize = SizeMap::Instance().ClassToSize(i);
      uint64_t nr_objects = sizeclass_histogram_[i];
      fprintf(fp_,
              "Class\t%5" PRIu64 "%2sB\t"
              "objects\t%5" PRIu64 "%2s\t"
              "used\t%5" PRIu64 "%2sB\t"
              "frag\t%5" PRIu64 "%2sB\t"
              "put\t%5" PRIu64 "%2s\t"
              "get\t%5" PRIu64 "%2s\t"
              "shrink\t%5" PRIu64 "%2s\t"
              "\n",
              GlobalProfiler::PrettySize(blocksize, s1), s1,
              GlobalProfiler::PrettySize(nr_objects, s2), s2,
              GlobalProfiler::PrettySize(nr_objects*blocksize, s3), s3,
              PrettySize(real_span_fragmentation_histogram_[i], s4), s4,
              PrettySize(spanpool_put_histogram_[i], s5), s5,
              PrettySize(spanpool_get_histogram_[i], s6), s6,
              PrettySize(spanpool_shrink_histogram_[i], s7), s7);
    }

    fprintf(fp_, "\n\n");
  }
} cache_aligned;  // class GlobalProfiler

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

    size_t size_class = SizeMap::SizeToClass(size);

    allocation_count_++;
    allocated_bytes_count_ += size;
    sizeclass_histogram_[size_class]++;

    if (size_class > 0 && size_class <= kNumClasses) {
      DecreaseRealSpanFragmentation(
          size_class, SizeMap::Instance().ClassToSize(size_class));
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
      IncreaseRealSpanFragmentation(
          size_class, SizeMap::Instance().ClassToSize(size_class));
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
  int64_t real_span_fragmentation_histogram_[kNumClasses+1];
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

  // and the cummulative values...
  uint64_t sizeclass_histogram_sum_[kNumClasses+1];
  int64_t real_span_fragmentation_histogram_sum_[kNumClasses+1];
  uint64_t block_stealing_count_sum_;
  uint64_t sizeclass_refill_count_sum_;
  uint64_t allocation_count_sum_;
  uint64_t allocated_bytes_count_sum_;
  uint64_t hot_free_count_sum_;
  uint64_t warm_free_count_sum_;
  uint64_t fast_free_count_sum_;
  uint64_t slow_free_count_sum_;
  uint64_t local_span_reuse_count_sum_;
  uint64_t remote_span_reuse_count_sum_;

  void Init() {
    if (!enabled_) return;
    self_allocating_ = true;
    tid_ = pthread_self();
    char filename[128];
    snprintf(filename, sizeof(filename), "memtrace-%lu", (long unsigned int)tid_);
    fp_ = fopen(filename, "w");
    if (fp_ == NULL) {
      ErrorOut("unable to open file %s", &filename);
    }
    self_allocating_ = false;
  }

  void PrintStatistics() {
    SCALLOC_PROFILER_METHOD_GUARD

    GlobalProfiler::Instance().UpdateGlobalState(
        sizeclass_histogram_,
        real_span_fragmentation_histogram_,
        block_stealing_count_,
        sizeclass_refill_count_,
        allocation_count_,
        allocated_bytes_count_,
        hot_free_count_,
        warm_free_count_,
        fast_free_count_,
        slow_free_count_,
        local_span_reuse_count_,
        remote_span_reuse_count_);

    for (unsigned i = 0; i < kNumClasses+1; ++i) {
      sizeclass_histogram_sum_[i] += sizeclass_histogram_[i];
      sizeclass_histogram_[i] = 0;
      real_span_fragmentation_histogram_sum_[i]
          += real_span_fragmentation_histogram_[i];
      real_span_fragmentation_histogram_[i] = 0;
    }
    block_stealing_count_sum_ += block_stealing_count_;
    sizeclass_refill_count_sum_ += sizeclass_refill_count_;
    allocation_count_sum_ += allocation_count_;
    allocated_bytes_count_sum_ += allocated_bytes_count_;
    hot_free_count_sum_ += hot_free_count_;
    warm_free_count_sum_ += warm_free_count_;
    fast_free_count_sum_ += fast_free_count_;
    slow_free_count_sum_ += slow_free_count_;
    local_span_reuse_count_sum_ += local_span_reuse_count_;
    remote_span_reuse_count_sum_ += remote_span_reuse_count_;

    block_stealing_count_ = 0;
    sizeclass_refill_count_ = 0;
    allocation_count_ = 0;
    allocated_bytes_count_ = 0;
    hot_free_count_ = 0;
    warm_free_count_ = 0;
    fast_free_count_ = 0;
    slow_free_count_ = 0;
    local_span_reuse_count_ = 0;
    remote_span_reuse_count_ = 0;


    uint64_t free_count = fast_free_count_sum_ + slow_free_count_sum_;
    fprintf(fp_,
            "Thread %lu; A %" PRIu64 "; R %" PRIu64 " "
            "BS %" PRIu64 " F %" PRIu64 " H %" PRIu64 " "
            "W %" PRIu64 " S %" PRIu64 " "
            "R/A %3.1f%%; "
            "BS/A %3.1f%%; "
            "H/FF %3.1f%%; "
            "W/FF %3.1f%%; "
            "S/F %3.1f%%; "
            "LSR/R %3.2f; "
            "RSR/R %3.2f; "
            "\n",
            tid_, allocation_count_sum_, sizeclass_refill_count_sum_,
            block_stealing_count_sum_, free_count, hot_free_count_sum_,
            warm_free_count_sum_, slow_free_count_sum_,
            100.0*static_cast<double>(sizeclass_refill_count_sum_) /
                static_cast<double>(allocation_count_sum_),
            100.0*static_cast<double>(block_stealing_count_sum_) /
                static_cast<double>(allocation_count_sum_),
            100.0*static_cast<double>(hot_free_count_sum_) /
                static_cast<double>(fast_free_count_sum_),
            100.0*static_cast<double>(warm_free_count_sum_) /
                static_cast<double>(fast_free_count_sum_),
            100.0*static_cast<double>(slow_free_count_sum_) /
                static_cast<double>(free_count),
            static_cast<double>(local_span_reuse_count_sum_) /
                static_cast<double>(sizeclass_refill_count_sum_),
            static_cast<double>(remote_span_reuse_count_sum_) /
                static_cast<double>(sizeclass_refill_count_sum_));

    fprintf(fp_, "SC-HISTO (and Frag): ");
    for (unsigned i = 1; i < kNumClasses + 1; ++i) {
      if (sizeclass_histogram_sum_[i] > 1000) {
        fprintf(fp_, "%d=%luk; ", i, sizeclass_histogram_sum_[i]/1000);
      } else {
        fprintf(fp_, "%d=%lu; ", i, sizeclass_histogram_sum_[i]);
      }
    }
    fprintf(fp_, "\n\n");
  }
} cache_aligned;  // class Profiler

}  // namespace scalloc

#endif  // SCALLOC_PROFILER_H_

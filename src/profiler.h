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

namespace scalloc {

class ProfilerInterface {
 public:
  virtual void LogAllocation(size_t size) = 0;
  virtual void LogDeallocation(size_t blocksize = 0, bool hot = true, bool remote = false) = 0;
  virtual void LogBlockStealing() = 0;
  virtual void LogSizeclassRefill() = 0;
};

class DummyProfiler : public ProfilerInterface {
 public:
  void LogAllocation(size_t size) {}
  void LogDeallocation(size_t blocksize = 0, bool hot = true, bool remote = false) {}
  void LogBlockStealing() {}
  void LogSizeclassRefill() {}
};

class Profiler : public ProfilerInterface {

 public:
  ~Profiler () {
    if (fp_) {
      fclose(fp_);
    }
  }
  static void Enable() { enabled_ = true; }
  static ProfilerInterface& GetProfiler();
  virtual void LogAllocation(size_t size) {
    if (UNLIKELY(fp_ == NULL)) Init();
    allocation_count_++;
    allocated_bytes_count_ += size;
    sizeclass_histogram_[Log2(size)]++;
    if (UNLIKELY(allocated_bytes_count_ >= kTimeQuantum_)) {
      allocated_bytes_count_ = 0;
      PrintStatistics();
    }
  }
  virtual void LogDeallocation(size_t blocksize = 0, bool hot = true, bool remote = false) {
    if (UNLIKELY(fp_ == NULL)) Init();
    if (remote) {
      slow_free_count_++;
    } else {
      fast_free_count_++;
      hot ? hot_free_count_++ : warm_free_count_++;
    }
  }
  virtual void LogBlockStealing() {
    block_stealing_count_++;
  }
  virtual void LogSizeclassRefill() {
    sizeclass_refill_count_++;
  }

 private:
  FILE *fp_;
  pthread_t tid_;
  uint32_t sizeclass_histogram_[33];  // 33 for everythong larger than 1<<32
  uint64_t block_stealing_count_;
  uint64_t sizeclass_refill_count_;
  uint64_t allocation_count_;
  uint64_t allocated_bytes_count_;
  uint64_t hot_free_count_;
  uint64_t warm_free_count_;
  uint64_t fast_free_count_;
  uint64_t slow_free_count_;

  static const size_t kTimeQuantum_ = 1UL << 20;
  static bool enabled_;

  void Init() {
    tid_ = pthread_self();
    char filename[128];
    snprintf(filename, 128, "memtrace-%lu", tid_);
    fp_ = fopen(filename, "w");
    if (fp_ == NULL) {
      ErrorOut("unable to open file %s", &filename);
    }
  }
  void PrintStatistics() {
    uint64_t free_count = fast_free_count_ + slow_free_count_;
    fprintf(fp_, "Thread %lu; A %lu; R %lu; BS %lu; F %lu; H %lu; W %lu; S %lu; "
            "R/A %3.1f%%; " 
            "BS/A %3.1f%%; " 
            "H/FF %3.1f%%; " 
            "W/FF %3.1f%%; " 
            "S/F %3.1f%%\n", 
            tid_, allocation_count_, sizeclass_refill_count_, block_stealing_count_,
            free_count,
            hot_free_count_, warm_free_count_, slow_free_count_,
            100.0*(double)sizeclass_refill_count_/(double)allocation_count_,
            100.0*(double)block_stealing_count_/(double)allocation_count_,
            100.0*(double)hot_free_count_/(double)fast_free_count_,
            100.0*(double)warm_free_count_/(double)fast_free_count_,
            100.0*(double)slow_free_count_/(double)free_count
            );
    for (unsigned i = 0; i < 33; ++i) {
      if (sizeclass_histogram_[i] > 1000)
        fprintf(fp_, "%d=%uk;", i, sizeclass_histogram_[i]/1000);
      else
        fprintf(fp_, "%d=%u;", i, sizeclass_histogram_[i]);
    }
    fprintf(fp_, "\n");
  }

} cache_aligned;

} //namespace scalloc
#endif  // SCALLOC_PROFILER_H_

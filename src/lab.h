// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_LAB_H_
#define SCALLOC_LAB_H_

#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include <atomic>

#include "core.h"
#include "core_id.h"
#include "globals.h"
#include "log.h"

namespace scalloc {

template<class T>
class TLSBase {
 protected:
  typedef void (*TLSDestructor)(void*);

  always_inline TLSBase(TLSDestructor dtor);
  always_inline void SetTLS(T* t);
  always_inline T* GetTLS();

 private:
  static TLS_ATTRIBUTE T* ab_ __attribute__((aligned(4 * 128)));
  pthread_key_t tls_key_;
};


template<class T>
TLS_ATTRIBUTE T* TLSBase<T>::ab_ = NULL;


template<class T>
TLSBase<T>::TLSBase(TLSDestructor dtor) {
  pthread_key_create(&tls_key_, dtor);
}


template<class T>
T* TLSBase<T>::GetTLS() {
#ifdef HAVE_TLS
  return ab_;
#else
  return static_cast<T*>(pthread_getspecific(tls_key_));
#endif  // HAVE_TLS
}


template<class T>
void TLSBase<T>::SetTLS(T* t) {
#ifdef HAVE_TLS
  ab_ = t;
#endif  // HAVE_TLS
  // Also set TSD to receive an argument in dtors.
  pthread_setspecific(tls_key_, t);
}


class ThreadLocalAllocationBuffer : public TLSBase<Core> {
 public:
  always_inline ThreadLocalAllocationBuffer();
  always_inline Core& GetAB();
  always_inline void GetMeALAB();

 private:
  typedef Stack<128> FreeAllocationBuffers;

  static inline void ThreadDestructor(void* tlab);

  always_inline Core* FindFreeAB();

  static std::atomic<int_fast32_t> thread_ids_ __attribute__((aligned(128)));
  static FreeAllocationBuffers free_abs_ __attribute__((aligned(128)));
};


std::atomic<int_fast32_t> ThreadLocalAllocationBuffer::thread_ids_;
Stack<128> ThreadLocalAllocationBuffer::free_abs_;


ThreadLocalAllocationBuffer::ThreadLocalAllocationBuffer()
    : TLSBase<Core>(ThreadDestructor) {
}


Core* ThreadLocalAllocationBuffer::FindFreeAB() {
  Core* ab = reinterpret_cast<Core*>(free_abs_.Pop());
  if (ab == NULL) {
    ab =  new(core_space.Allocate(sizeof(Core))) Core();
  }
  return ab;
}


void ThreadLocalAllocationBuffer::ThreadDestructor(void* tlab) {
  LOG(kTrace, "Destroy at %p", tlab);
  reinterpret_cast<Core*>(tlab)->Destroy();
  span_pool.AnnounceLeavingThread();
  free_abs_.Push(tlab);
}


void ThreadLocalAllocationBuffer::GetMeALAB() {
  Core* ab = GetTLS();
  if (LIKELY(ab == NULL)) {
    ab = FindFreeAB();
    if (UNLIKELY(ab == NULL)) {
      Fatal("reached maximum number of threads.");
    }
    ab->Init(core_id(ab, thread_ids_.fetch_add(1) + 1));
    SetTLS(ab);
    span_pool.AnnounceNewThread();
  }
}

Core& ThreadLocalAllocationBuffer::GetAB() {
  Core* ab = GetTLS();
  /*
  if (UNLIKELY(ab == NULL)) {
    Fatal("should not happen!");

    ab = FindFreeAB();
    if (UNLIKELY(ab == NULL)) {
      Fatal("reached maximum number of threads.");
    }
    LOG(kTrace, "New at %p", ab);
    ab->Init(core_id(ab, thread_ids_.fetch_add(1) + 1));
    SetTLS(ab);
    span_pool.AnnounceNewThread();
  }
  */
  return *ab;
}


class RoundRobinAllocationBuffer : public TLSBase<GuardedCore> {
 public:
  always_inline RoundRobinAllocationBuffer();
  always_inline GuardedCore& GetAB();

 private:
  static inline void ThreadDestructor(void* lab);

  static GuardedCore allocation_buffers_[kMaxThreads];
  std::atomic<uint_fast64_t> thread_counter_;
};


GuardedCore RoundRobinAllocationBuffer::allocation_buffers_[kMaxThreads];


RoundRobinAllocationBuffer::RoundRobinAllocationBuffer()
    : TLSBase<scalloc::GuardedCore>(ThreadDestructor)
    , thread_counter_(0) {
}


void RoundRobinAllocationBuffer::ThreadDestructor(void* lab) {
}


GuardedCore& RoundRobinAllocationBuffer::GetAB() {
  GuardedCore* ab = GetTLS();
  if (UNLIKELY(ab == NULL)) {
    int32_t num_cores = CpusOnline();
    ab = &allocation_buffers_[thread_counter_.fetch_add(1) % num_cores];
    SetTLS(ab);
    LOG(kTrace, "RoundRobinAllocationBuffer: tid: %lu", thread_counter_.load());
    ab->AnnounceNewThread();
    while (ab->InUse()) { __asm__("PAUSE"); }
    LOG(kTrace, "thread_counter_ %lu up and ready", thread_counter_.load());
  }
  return *ab;
}

}  // namespace scalloc

#endif  // SCALLOC_LAB_H_


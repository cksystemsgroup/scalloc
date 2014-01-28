// Copyright (c) 2014, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(__APPLE__)

#include <mach/mach_init.h>
#include <mach/task.h>

#endif  // __APPLE__

#ifdef __linux__

#include <sys/resource.h>

#endif  // __linux__

#define ALLOCATIONS 1000000
#define SIZE 8


#if defined(__APPLE__)

static size_t GetRSS() {
  struct task_basic_info info;
  mach_msg_type_number_t info_count = TASK_BASIC_INFO_COUNT;
  task_info(current_task(), TASK_BASIC_INFO, (task_info_t) &info, &info_count);
  return (size_t) info.resident_size;
}

#endif  // __APPLE__

#ifdef __linux__

static size_t GetRSS() {
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_maxrss;
}

#endif


static void* ProducerWorkload(void* data) {
  printf("producing\n");
  void** memory = (void**) data;
  for (size_t i = 0; i < ALLOCATIONS; i++) {
    memory[i] = malloc(SIZE);
  }
  return NULL;
}


static void* ConsumerWorkload(void* data) {
  printf("consuming\n");
  void** memory = (void**) data;
  for (size_t i = 0; i < ALLOCATIONS; i++) {
    free(memory[i]);
  }
  return NULL;
}


int main(int argc, char** argv) {
  int iterations = 1000;
  void* memory[ALLOCATIONS];
  pthread_t producer;
  pthread_t consumer;
  
  setbuf(stdout, NULL);
  
  for (int i = 0; i < iterations; i++) {
    if (pthread_create(&producer, NULL, ProducerWorkload, (void*)memory) != 0) {
      perror("pthread_create");
      abort();
    }
    pthread_join(producer, NULL);
    
    if (pthread_create(&consumer, NULL, ConsumerWorkload, (void*)memory) != 0) {
      perror("pthread_create");
      abort();
    }
    pthread_join(consumer, NULL);
    
    printf("rss: %lu\n", GetRSS());
  }
  
  return EXIT_SUCCESS;
}

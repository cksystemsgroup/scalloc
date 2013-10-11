// Copyright (c) 2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_OVERRIDE_OSX_H_
#define SCALLOC_OVERRIDE_OSX_H_

// For more information on this magic, see
//  https://code.google.com/p/gperftools/source/browse/src/libc_override_osx.h
//  http://www.opensource.apple.com/source/Libc/Libc-583/include/malloc/malloc.h
//  http://www.opensource.apple.com/source/Libc/Libc-583/gen/malloc.c
//
// We only support OS X > 10.6.

#if !defined(__APPLE__)
#error override_osx.h is for OS X only.
#endif

#include <AvailabilityMacros.h>
#include <malloc/malloc.h>

#include "scalloc.h"

#if !defined(MAC_OS_X_VERSION_10_6)
#error Override only working for OS X 10.6 and later.
#endif

namespace {

kern_return_t mi_enumerator(task_t task, void *,
                            unsigned type_mask, vm_address_t zone_address,
                            memory_reader_t reader,
                            vm_range_recorder_t recorder) {
  return KERN_FAILURE;
}
  
size_t mi_good_size(malloc_zone_t *zone, size_t size) {
  return size;
}
  
boolean_t mi_check(malloc_zone_t *zone) {
  return true;
}
  
void mi_print(malloc_zone_t *zone, boolean_t verbose) {}

void mi_log(malloc_zone_t *zone, void *address) {}
  
void mi_force_lock(malloc_zone_t *zone) {}

void mi_force_unlock(malloc_zone_t *zone) {}

boolean_t mi_zone_locked(malloc_zone_t *zone) {
  return false;
}

void* mz_malloc(malloc_zone_t* zone, size_t size) {
  printf("In mz_malloc()\n");
  return scalloc::malloc(size);
}

void mz_free(malloc_zone_t* zone, void* ptr) {
  printf("in mz_free\n");
  return scalloc::free(ptr);
}

void* mz_calloc(malloc_zone_t* zone, size_t num_items, size_t size) {
  return scalloc::calloc(num_items, size);
}

void* mz_valloc(malloc_zone_t* zone, size_t size) {
  return scalloc::valloc(size);
}

size_t mz_size(malloc_zone_t* zone, const void* ptr) {
  if (!scalloc::Ours(ptr)) {
    return 0;
  }
  return 1; // TODO
}

void* mz_realloc(malloc_zone_t* zone, void* ptr, size_t size) {
  return scalloc::realloc(ptr, size);
}

void* mz_memalign(malloc_zone_t* zone, size_t align, size_t size) {
  return scalloc::memalign(align, size);
}

void mz_destroy(malloc_zone_t* zone) {
  // A no-op -- we will not be destroyed!
}

}

extern "C" {
  void cfree(void* p) { scalloc::free(p); }
}

extern "C" {
  // This function is only available on 10.6 (and later) but the
  // LibSystem headers do not use AvailabilityMacros.h to handle weak
  // importing automatically.  This prototype is a copy of the one in
  // <malloc/malloc.h> with the WEAK_IMPORT_ATTRBIUTE added.
  extern malloc_zone_t *malloc_default_purgeable_zone(void)
  WEAK_IMPORT_ATTRIBUTE;
}

static void ReplaceSystemAlloc() {
  static malloc_introspection_t scalloc_introspection;
  memset(&scalloc_introspection, 0, sizeof(scalloc_introspection));
  
  scalloc_introspection.enumerator = &mi_enumerator;
  scalloc_introspection.good_size = &mi_good_size;
  scalloc_introspection.check = &mi_check;
  scalloc_introspection.print = &mi_print;
  scalloc_introspection.log = &mi_log;
  scalloc_introspection.force_lock = &mi_force_lock;
  scalloc_introspection.force_unlock = &mi_force_unlock;
  scalloc_introspection.zone_locked = &mi_zone_locked;
  
  static malloc_zone_t scalloc_zone;
  memset(&scalloc_zone, 0, sizeof(scalloc_zone));
  
  scalloc_zone.version = 6;
  scalloc_zone.zone_name = "scalloc";
  scalloc_zone.size = &mz_size;
  scalloc_zone.malloc = &mz_malloc;
  scalloc_zone.valloc = &mz_valloc;
  scalloc_zone.calloc = &mz_calloc;
  scalloc_zone.free = &mz_free;
  scalloc_zone.memalign = &mz_memalign;
  scalloc_zone.realloc = &mz_realloc;
  scalloc_zone.destroy = &mz_destroy;
  scalloc_zone.introspect = &scalloc_introspection;
  
  scalloc_zone.batch_malloc = NULL;
  scalloc_zone.batch_free = NULL;
  scalloc_zone.free_definite_size = NULL;
  
  if (malloc_default_purgeable_zone) {
    malloc_default_purgeable_zone();
  }
  
  malloc_zone_register(&scalloc_zone);
  malloc_zone_t* default_zone = malloc_default_zone();
  malloc_zone_unregister(default_zone);
  malloc_zone_register(default_zone);
}

#endif  // SCALLOC_OVERRIDE_OSX_H_

// Copyright (c) 2015, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#if !defined(__APPLE__)
#error override_osx.h should be included on OSX only.
#endif  // __APPLE__

#include <AvailabilityMacros.h>
#include <malloc/malloc.h>

#if !defined(MAC_OS_X_VERSION_10_6)
#error "only osx>=10.6 supported."
#endif  // MAC_OS_X_VERSION_10_6

#include "log.h"
#include "glue.h"
#include "platform/globals.h"

namespace scalloc {
namespace zoned {

kern_return_t mi_enumerator(task_t task,
                            void* ,
                            unsigned type_mask,
                            vm_address_t zone_address,
                            memory_reader_t reader,
                            vm_range_recorder_t recorder) {
  return KERN_FAILURE;
}


size_t mi_good_size(malloc_zone_t* zone, size_t size) {
  return size;
}


boolean_t mi_check(malloc_zone_t* zone) {
  return true;
}


void mi_print(malloc_zone_t* zone, boolean_t verbose) {
}


void mi_log(malloc_zone_t* zone, void* address) {
}


void mi_force_lock(malloc_zone_t* zone) {
}


void mi_force_unlock(malloc_zone_t* zone) {
}


boolean_t mi_zone_locked(malloc_zone_t* zone) {
  return false;
}


void* mz_malloc(malloc_zone_t* zone, size_t size) {
  return scalloc::malloc(size);
}


void mz_free(malloc_zone_t* zone, void* p) {
  scalloc::free(p);
}


void* mz_calloc(malloc_zone_t* zone, size_t num_items, size_t size) {
  return scalloc::calloc(num_items, size);
}


void* mz_valloc(malloc_zone_t* zone, size_t size) {
  return scalloc::valloc(size);
}


size_t mz_size(malloc_zone_t* zone, const void* p) {
  LOG(kTrace, "mz_size for ptr: %p", p);
  Span* s;
  if (LIKELY(object_space.Contains(p) && (s = Span::FromObject(p)))) {
    return ClassToSize[s->size_class()];
  }
  // TODO: large objects.
  return 0;
}


void* mz_realloc(malloc_zone_t* zone, void* p, size_t size) {
  return scalloc::realloc(p, size);
}


void* mz_memalign(malloc_zone_t* zone, size_t align, size_t size) {
  return scalloc::memalign(align, size);
}


void mz_destroy(malloc_zone_t* zone) {
  // Not going to happen!
}

}  // namespace zoned
}  // namespace scalloc

extern "C" {
void cfree(void* p) {
  scalloc::free(p);
}

// This function is only available on 10.6 (and later) but the
// LibSystem headers do not use AvailabilityMacros.h to handle weak
// importing automatically.  This prototype is a copy of the one in
// <malloc/malloc.h> with the WEAK_IMPORT_ATTRBIUTE added.
extern malloc_zone_t* malloc_default_purgeable_zone(void) WEAK_IMPORT_ATTRIBUTE;
}

namespace scalloc {

always_inline void ReplaceSystemAllocator() {
  static malloc_introspection_t scalloc_introspection;
  memset(&scalloc_introspection, 0, sizeof(scalloc_introspection));
  scalloc_introspection.enumerator = &zoned::mi_enumerator;
  scalloc_introspection.good_size = &zoned::mi_good_size;
  scalloc_introspection.check = &zoned::mi_check;
  scalloc_introspection.print = &zoned::mi_print;
  scalloc_introspection.log = & zoned::mi_log;
  scalloc_introspection.force_lock = &zoned::mi_force_lock;
  scalloc_introspection.force_unlock = &zoned::mi_force_unlock;
  scalloc_introspection.zone_locked = &zoned::mi_zone_locked;

  static malloc_zone_t scalloc_zone;
  memset(&scalloc_zone, 0, sizeof(scalloc_zone));
  scalloc_zone.version = 6;
  scalloc_zone.zone_name = "scalloc";
  scalloc_zone.size = &zoned::mz_size;
  scalloc_zone.malloc = &zoned::mz_malloc;
  scalloc_zone.calloc = &zoned::mz_calloc;
  scalloc_zone.valloc = &zoned::mz_valloc;
  scalloc_zone.free = &zoned::mz_free;
  scalloc_zone.memalign = &zoned::mz_memalign;
  scalloc_zone.realloc = &zoned::mz_realloc;
  scalloc_zone.destroy = &zoned::mz_destroy;
  scalloc_zone.introspect = &scalloc_introspection;

  if (malloc_default_purgeable_zone) {
    malloc_default_purgeable_zone();
  }

  malloc_zone_register(&scalloc_zone);
  malloc_zone_t* default_zone = malloc_default_zone();
  malloc_zone_unregister(default_zone);
  malloc_zone_register(default_zone);
}

}  // namespace scalloc

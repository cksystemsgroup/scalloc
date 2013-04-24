#include "scalloc.h"

#include "common.h"

namespace scalloc {

always_inline void* malloc(const size_t size) {
  return NULL;
}

always_inline void free(void* p) {
  if (UNLIKELY(p == NULL)) {
    return;
  }
  return;
}

}  // namespace scalloc

extern "C" void* scalloc_malloc(size_t size) __THROW {
  return scalloc::malloc(size);
}

extern "C" void scalloc_free(void* p) __THROW {
  scalloc::free(p);
}

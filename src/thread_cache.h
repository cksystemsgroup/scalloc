#ifndef SCALLOC_THREAD_CACHE_H_
#define SCALLOC_THREAD_CACHE_H_

#include "allocators/slab_sc_allocator.h"
#include "block_header.h"
#include "common.h"

namespace scalloc {

// A threadlocal cache that may hold any allocator.  This interface can be used
// globally, as everything will initialize itself.
class ThreadCache {
 public:
  static void InitModule();

  static ThreadCache& GetCache();

  void* Allocate(const size_t size);
  void Free(void* ptr, BlockHeader* hdr);

 private:
  static __thread ThreadCache* tl_cache_ TLS_MODE;
  static bool module_init_;

  static ThreadCache* NewCache();

  SlabScAllocator allocator_;
} cache_aligned;

always_inline ThreadCache& ThreadCache::GetCache() {
  if (LIKELY(tl_cache_ != NULL)) {
    return *tl_cache_; 
  }
  // We cannot trust that we are initialized, so do this first.
  if (!module_init_) {
    InitModule();
  }
  // We don't have a cache yet, so let's create one.
  tl_cache_ = NewCache();
  return *tl_cache_;
}

always_inline void* ThreadCache::Allocate(const size_t size) {
  return allocator_.Allocate(size);
}

always_inline void ThreadCache::Free(void* p, BlockHeader* hdr) {
  allocator_.Free(p, reinterpret_cast<SlabHeader*>(hdr));
}

}  // namespace scalloc

#endif  // SCALLOC_THREAD_CACHE_H_

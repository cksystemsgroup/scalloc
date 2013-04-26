// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "slab_sc_allocator.h"

#include "allocators/page_heap.h"
#include "log.h"
#include "runtime_vars.h"

namespace scalloc {

void SlabScAllocator::InitModule() {
  PageHeap::InitModule();
}

void SlabScAllocator::Init(const uint64_t id) {
  id_ = id;
}

void* SlabScAllocator::AllocateNoSlab(const size_t sc, const size_t size) {
  // Size class 0 represents an object of size 0, which results in malloc()
  // returning NULL.
  if (sc == 0) {
    return NULL;
  }

  if (my_headers_[sc] != NULL) {
    // Only try to steal we had a slab at least once.
    // TODO(mlippautz): stealing ;)
  }

  Refill(sc);
  return Allocate(size);
}

void SlabScAllocator::Refill(const size_t sc) {
  uintptr_t block = reinterpret_cast<uintptr_t>(PageHeap::GetHeap()->Get());
  if (block == 0) {
    ErrorOut("PageHeap out of memory");
  }
  SlabHeader* hdr = InitSlab(block, kPageMultiple * RuntimeVars::SystemPageSize(), sc);
  SetActiveSlab(sc, hdr);
}

SlabHeader* SlabScAllocator::InitSlab(uintptr_t block,
                                      size_t len,
                                      const size_t sc) {
  const size_t obj_size = SizeMap::Instance().ClassToSize(sc);
  size_t sys_page_size = RuntimeVars::SystemPageSize();
  SlabHeader* main_hdr = reinterpret_cast<SlabHeader*>(block);
  main_hdr->Reset(sc, id_);
  main_hdr->owner = id_;
  main_hdr->active = true;

  // We need to initialize the flist and place ForwardHeaders every system page
  // size bytes, since this is where we search for headers (for small AND large
  // objects)

  size_t nr_objects;

  // First block is special, since it contains the full slab header, while
  // others only contain the forward header to the first block.
  nr_objects = (sys_page_size - sizeof(*main_hdr)) / obj_size;
  main_hdr->flist.FromBlock(reinterpret_cast<void*>(block + sizeof(*main_hdr)),
                            obj_size,
                            nr_objects);
  len -= sys_page_size;
  block += sys_page_size;

  // Fill with forward headers and flist objects.
  for (; len >= sys_page_size; len -= sys_page_size) {
    ForwardHeader* fwd_hdr = reinterpret_cast<ForwardHeader*>(block);
    fwd_hdr->Reset(main_hdr);
    nr_objects = (sys_page_size - sizeof(*fwd_hdr)) / obj_size;
    main_hdr->flist.AddRange(reinterpret_cast<void*>(block + sizeof(*fwd_hdr)),
                             obj_size,
                             nr_objects);
    block += sys_page_size;
  }

  return main_hdr;
}

void SlabScAllocator::RemoteFree(void* p, SlabHeader* hdr) {
  // TODO(mlippautz): handle remote frees
}

}  // namespace scalloc

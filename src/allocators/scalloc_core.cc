#include "scalloc_core-inl.h"


namespace scalloc {

TypedAllocator<ScallocCore>* ScallocCore::allocator;

bool ScallocCore::enabled_;

}  // namespace scalloc

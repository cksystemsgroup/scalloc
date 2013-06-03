// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/medium_allocator.h"

namespace scalloc {

bool MediumAllocator::enabled_;
SpinLock MediumAllocator::lock_;
DList<HalfFit*> MediumAllocator::list_;

}  // namespace scalloc

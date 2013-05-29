// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "allocators/medium_size_allocator.h"

bool MediumSizeAllocator::enabled_;
GlobalSbrkAllocator MediumSizeAllocator::arena_;
SpinLock MediumSizeAllocator::lock_;
DList<HalfFit*> MediumSizeAllocator::list_;

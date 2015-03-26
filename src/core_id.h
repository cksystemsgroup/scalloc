// Copyright (c) 2015, the scalloc project authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef SCALLOC_CORE_ID_H_
#define SCALLOC_CORE_ID_H_

#include "atomic_value.h"

namespace scalloc {

class Core;

// Pass by value!
typedef TaggedValue<Core*> core_id;

// Do not pass at all!
typedef AtomicTaggedValue<Core*, 0, 0> AtomicCoreID;

__attribute__((aligned(64))) const core_id kTerminated(nullptr, 0);

}  // namespace scalloc

#endif  // SCALLOC_CORE_ID_H_


// Copyright (c) 2012-2013, the scalloc Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#include "profiler.h"
#include "thread_cache.h"

namespace scalloc {

Profiler dummy;
bool Profiler::enabled_;

Profiler& Profiler::GetProfiler() {
#ifdef PROFILER_ON
  if (LIKELY(enabled_)) {
    return ThreadCache::GetCache().GetProfiler();
  } else {
    return dummy;
  }
#else
  return dummy;
#endif  // PROFILER_ON
}

}  // namespace scalloc

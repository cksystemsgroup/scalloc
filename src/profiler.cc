#include "profiler.h"
#include "thread_cache.h"

namespace scalloc {

Profiler dummy;
bool Profiler::enabled_;

Profiler& Profiler::GetProfiler() {
  if (LIKELY(enabled_)) {
    return ThreadCache::GetCache().GetProfiler();
  } else {
    return dummy;
  }
}

}  // namespace scalloc

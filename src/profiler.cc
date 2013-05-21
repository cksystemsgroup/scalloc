#ifdef PROFILER_ON
#include "profiler.h"
#include "thread_cache.h"

namespace scalloc {

DummyProfiler dummy;
bool Profiler::enabled_;

ProfilerInterface& Profiler::GetProfiler() {
  if (LIKELY(enabled_))
    return ThreadCache::GetCache().GetProfiler();
  else
    return dummy;
}

}  // namespace scalloc
#endif //  PROFILER_ON

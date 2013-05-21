#include "thread_cache.h"
//#include "profiler.h"

namespace scalloc {

Profiler& Profiler::GetProfiler() {
  return ThreadCache::GetCache().GetProfiler();
}


}  // namespace scalloc

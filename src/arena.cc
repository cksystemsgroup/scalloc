#include "arena.h"

#include "common.h"

GlobalSbrkAllocator SmallArena; 

void InitArenas() {
  SmallArena.Init(kSmallSpace);
}


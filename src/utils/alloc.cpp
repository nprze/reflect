#include "alloc.h"
#include <stdlib.h>

void* rfct::arenaAllocation::allocMemoryArena(size_t size) {
	RFCT_ASSERT(lastOffsetBytes+size<=fullSize);
	lastOffsetBytes += size;
	return ((char*)memory) + lastOffsetBytes - size;
}

rfct::arenaAllocation rfct::createArena(size_t size) {
	arenaAllocation alloc;
	alloc.memory = malloc(size);
	alloc.fullSize = size;
	alloc.lastOffsetBytes = 0;
	return alloc;
}

void rfct::deleteArena(arenaAllocation* arena) {
	free(arena->memory);
}

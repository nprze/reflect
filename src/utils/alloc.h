#pragma once

namespace rfct {
	struct arenaAllocation {
		void* memory;
		size_t lastOffsetBytes;
		size_t fullSize;
		void* allocMemoryArena(size_t size);
	};
	arenaAllocation createArena(size_t size);
	void deleteArena(arenaAllocation* arena);
}
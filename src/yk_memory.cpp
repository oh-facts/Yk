#include <yk_memory.h>


void yk_memory_arena_innit(YkMemoryArena* arena, size_t size, void* base)
{
	arena->size = size;
	arena->base = (u8*)base;
	arena->used = 0;
}

void yk_memory_arena_clean_reset(YkMemoryArena* arena)
{
	memset(arena->base, 0, arena->used);
	arena->used = 0;
}

YkMemoryArena yk_memory_sub_arena(YkMemoryArena* arena, size_t size)
{
	Assert(arena->size - arena->used >= size, "out of space");
	YkMemoryArena out = {};
	out.base = (u8*)arena->base + arena->used;
	arena->used += size;
	out.size = size;

	return out;
}
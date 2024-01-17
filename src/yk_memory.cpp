#include <yk_memory.h>
#include <iostream>

void yk_memory_arena_innit(YkMemoryArena* arena, size_t size, void* base)
{
	arena->size = size;
	arena->base = (u8*)base;
	arena->used = 0;
}

void yk_memory_arena_push(YkMemoryArena* arena, size_t size, void* data)
{
	Assert(size < arena->size - arena->used, "too much memory");
	Assert(data!=0, "data is null")
	memcpy(arena->base + arena->used, data, size);
	arena->used += size;
}
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
	Assert(data != 0, "data is null")
	Assert(size < arena->size - arena->used, "too much memory");
	memcpy(arena->base + arena->used, data, size);
	arena->used += size;
}

void yk_memory_arena_insert(YkMemoryArena* arena, size_t size, size_t index, void* data)
{
	Assert(data != 0, "data is null")
	//Assert(index * size < arena->size - arena->used, "too much memory");

	memcpy(arena->base + index * size, data, size);

}

void yk_memory_arena_zero(YkMemoryArena* arena)
{
	memset(arena->base, 0, arena->used);
}

YkMemoryArena yk_memory_sub_arena(YkMemoryArena* arena, size_t size)
{
	YkMemoryArena out = {};
	out.base = (u8*)arena->base + arena->used;
	arena->used += size;
	out.size = size;

	return out;
}
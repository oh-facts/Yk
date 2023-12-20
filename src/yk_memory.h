#ifndef YK_MEMORY
#define YK_MEMORY

#include <defines.h>

typedef struct YkMemoryArena YkMemoryArena;

struct YkMemoryArena
{
	size_t size;
	u8* base;
	size_t used;
};

void yk_memory_arena_innit(YkMemoryArena* arena, size_t size, u8* base)
{
	arena->size = size;
	arena->base = base;
	arena->used = 0;
}
#endif // !YK_MEMORY

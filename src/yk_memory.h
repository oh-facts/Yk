#ifndef YK_MEMORY
#define YK_MEMORY

#include <defines.h>
#include <Windows.h>
#include <yk_api.h>

struct YkMemoryArena
{
	size_t size;
	u8* base;
	size_t used;
};

struct YkMemory
{
    int is_initialized;

    YkMemoryArena perm_storage;
    YkMemoryArena temp_storage;
};

typedef struct YkMemory YkMemory;


YK_API void yk_memory_arena_innit(YkMemoryArena* arena, size_t size, void* base);
YK_API void yk_memory_arena_push(YkMemoryArena* arena, size_t size, void* data);
YK_API void yk_memory_arena_zero(YkMemoryArena* arena);
YK_API YkMemoryArena yk_memory_sub_arena(YkMemoryArena* arena, size_t size);
#endif // !YK_MEMORY

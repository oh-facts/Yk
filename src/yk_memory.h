#ifndef YK_MEMORY
#define YK_MEMORY

#include <defines.h>
#include <Windows.h>

typedef struct YkMemoryArena YkMemoryArena;

struct YkMemory
{
    int is_initialized;
    u64 perm_storage_size;
    void* perm_storage;
    u64 temp_storage_size;
    void* temp_storage;
};

typedef struct YkMemory YkMemory;


struct YkMemoryArena
{
	size_t size;
	u8* base;
	size_t used;
};

void yk_memory_arena_innit(YkMemoryArena* arena, size_t size, void* base);
void yk_memory_arena_push(YkMemoryArena* arena, size_t size, void* data);

#endif // !YK_MEMORY

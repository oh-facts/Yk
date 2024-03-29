#ifndef YKR_MODEL_LOADER_H
#define YKR_MODEL_LOADER_H
#include <renderer/ykr_common.h>

struct load_mesh_scratch_arena
{
    YkMemoryArena vertices;
    YkMemoryArena indices;
};

/*
	Scratch arena can be dumped.
	perm arena needs to persist.
	out_num_meshes stores the 
	number of meshes.
*/
void ykr_load_mesh(YkRenderer* renderer,  const char* filepath, load_mesh_scratch_arena* scratch, model_assets* model);

/*
	Clear mesh loading context. Uses local static variables to make mesh loading non-convulated
*/
void ykr_load_mesh_cleanup();

/*
    google.com
*/
inline unsigned long djb2_hash(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}


#endif // !YKR_MODEL_LOADER_H

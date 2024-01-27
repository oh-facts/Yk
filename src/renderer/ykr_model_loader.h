#ifndef YKR_MODEL_LOADER_H
#define YKR_MODEL_LOADER_H
#include <renderer/ykr_common.h>

/*
	Scratch arena can be dumped.
	perm arena needs to persist.
	out_num_meshes stores the 
	number of meshes.
*/
mesh_asset* ykr_load_mesh(YkRenderer* renderer, const char* filepath, YkMemoryArena* scratch, YkMemoryArena* perm, size_t* out_num_meshes);

#endif // !YKR_MODEL_LOADER_H

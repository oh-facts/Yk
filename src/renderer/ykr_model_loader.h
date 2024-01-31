#ifndef YKR_MODEL_LOADER_H
#define YKR_MODEL_LOADER_H
#include <renderer/ykr_common.h>

/*
	Scratch arena can be dumped.
	perm arena needs to persist.
	out_num_meshes stores the 
	number of meshes.
*/
mesh_asset* ykr_load_mesh(const YkRenderer* renderer, const char* filepath, YkMemoryArena* scratch, YkMemoryArena* perm, size_t* num_mesh);

/*
	Clear mesh loading context. Uses local static variables to make mesh loading non-convulated
*/
void ykr_load_mesh_cleanup();

#endif // !YKR_MODEL_LOADER_H

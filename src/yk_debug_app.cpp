#include <yk_debug_app.h>

#include <yk.h>

/*
    I hope no one sees this. I don't want to be casted to the deep circles of hell
*/

#define dmon     "res/models/doraemon/doraemon.glb"   
#define shinchan "res/models/shinchan/scene.glb"
#define fits     "res/models/fire_in_the_sky/scene.gltf"
#define fits2    "res/models/fire_in_the_sky2/scene.gltf"
#define sponza   "res/models/sponza/sponza.gltf"
#define duck     "res/models/duck/Duck.gltf"
#define msa      "res/models/mystery_shack_attic/mystery_shack_attic.glb"
#define doppio   "res/models/doppio/scene.gltf"
#define tr       "res/models/test_room/scene.glb"
#define room     "res/models/room/scene.gltf"
#define bill     "res/models/bill/scene1.glb"
#define marc     "res/models/marceline/scene.gltf"


/*
    Note: shinchan needs material 1
          fits, fits2 need material 0
          rest need debug_color 1
          (check ykr_model_loader.cpp)
*/


#if DEBUG
LPVOID base_address = (LPVOID)Terabytes(2);
#else
LPVOID base_address = 0;
#endif

void engine_memory_innit(YkMemory* engine_memory)
{
    size_t perm_storage_size = Megabytes(64);
    size_t temp_storage_size = Gigabytes(1);

    u64 total_size = perm_storage_size + temp_storage_size;

    yk_memory_arena_innit(&engine_memory->perm_storage, perm_storage_size, VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    yk_memory_arena_innit(&engine_memory->temp_storage, temp_storage_size, (u8*)engine_memory->perm_storage.base + perm_storage_size);

    engine_memory->is_initialized = 1;
}

void engine_memory_cleanup(YkMemory* engine_memory)
{
    yk_memory_arena_clean_reset(&engine_memory->perm_storage);
    yk_memory_arena_clean_reset(&engine_memory->temp_storage);
    engine_memory->is_initialized = 0;
    VirtualFree(base_address, 0, MEM_RELEASE);
}

void set_obj_pos(mesh_asset* asset, u32 mesh_count, glm::vec3 pos, f32 angle, glm::vec3 rot, glm::vec3 scale)
{
    for (u32 i = 0; i < mesh_count; i++)
    {
        glm::mat4 temp = glm::mat4(1);

        temp = glm::translate(temp, pos);
        temp = glm::rotate(temp, angle , rot);
        temp = glm::scale(temp, scale);

        asset[i].model_mat = temp * asset[i].model_mat;

    }
}

#define obj_count 3

void load_scene(YkRenderer* ren, const char* paths[], mesh_asset* assets[], size_t sizes[], YkMemoryArena* scratch, YkMemoryArena* perm )
{
    for(u32 i = 0; i < obj_count; i ++)
    {
        assets[i] = ykr_load_mesh(ren, paths[i],scratch, perm, &sizes[i]);
    }
}

size_t size_sum(size_t sizes[])
{
    size_t out = 0;
    for(u32 i = 0; i < obj_count; i ++ )
    {
        out += sizes[i];
    }
    return out;
}

YK_API void _debug_app_start(struct YkDebugAppState* self)
{

    engine_memory_innit(&self->engine_memory);
    yk_renderer_innit(&self->ren, &self->window);
    self->ren.texture_count = 0;
    size_t model_load_size = Gigabytes(1);
    YkMemoryArena scratch = yk_memory_sub_arena(&self->engine_memory.temp_storage, model_load_size);

    size_t model_load_temp = Megabytes(5);
    YkMemoryArena perm_sub = yk_memory_sub_arena(&self->engine_memory.perm_storage, model_load_temp);

    self->ren.textures = yk_memory_sub_arena(&self->engine_memory.perm_storage, Megabytes(1));

    const char* asset_paths[obj_count] = {
        room,
        shinchan,
    //    marc,
        bill
    };

    mesh_asset* assets[obj_count] = {};
    size_t sizes[obj_count] = {};


    load_scene(&self->ren, asset_paths,assets,sizes,&scratch,&perm_sub);

    yk_memory_arena_clean_reset(&self->engine_memory.temp_storage);

    set_obj_pos(assets[1], sizes[1], glm::vec3(-32, -31, -9), 90 * DEG_TO_RAD, glm::vec3(0, 1, 0), glm::vec3(0.5f));
    set_obj_pos(assets[2], sizes[2], glm::vec3(-32, -29.7, -12), 90 * DEG_TO_RAD, glm::vec3(0, 1, 0), glm::vec3(0.5f));
   // set_obj_pos(assets[3], sizes[3], glm::vec3(-16, -26.6f, -10.5f), -90 * DEG_TO_RAD, glm::vec3(0, 1, 0), glm::vec3(0.5f));
    
    self->ren.test_meshes = assets[0];
    self->ren.test_mesh_count = size_sum(sizes);
 

    self->ren.cam.pos = glm::vec3{ -6.51f, -30.31f,-10.13f };
    self->ren.cam.yaw = -1.6f;

    yk_renderer_innit_scene(&self->ren);

    yk_recreate_swapchain(&self->ren, &self->window);

}

YK_API void _debug_app_update(struct YkDebugAppState* self, f64 dt)
{
    yk_renderer_draw(&self->ren, &self->window, dt);
   
    static int a = 1;
    if(a == 0)
    {
        static f32 time = 0;
        for(u32 i = 0; i < self->ren.test_mesh_count; i ++)
        {
            glm::mat4 mat = glm::mat4(1);
            mat = glm::rotate( mat, (f32)dt , glm::vec3(0,1,0));

            self->ren.test_meshes[i].model_mat = mat * self->ren.test_meshes[i].model_mat;
        }

    }
}

YK_API int _debug_app_is_running(struct YkDebugAppState* self)
{
    return self->window.win_data.is_running;
}

YK_API void _debug_app_update_references(struct YkDebugAppState* self)
{
   yk_recreate_swapchain(&self->ren, &self->window);
   // printf("%p", self->window.hinstance);
    //yk_renderer_innit(&self->ren, &self->window);
}

YK_API void _debug_app_shutdown(struct YkDebugAppState* self)
{
    yk_renderer_wait(&self->ren);
    yk_free_renderer(&self->ren);
    engine_memory_cleanup(&self->engine_memory);
}

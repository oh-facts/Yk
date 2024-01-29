#include <yk_debug_app.h>
#include <math.h>
#include <yk.h>

/*
    I hope no one sees this. I don't want to be casted to the deep circles of hell
*/

#define shinchan "res/models/shinchan/shinchan.gltf"
#define fits     "res/models/fire_in_the_sky/scene.gltf"
#define fits2    "res/models/fire_in_the_sky2/scene.gltf"
#define sponza   "res/models/sponza/sponza.gltf"
#define duck     "res/models/duck/Duck.gltf"
#define msa      "res/models/mystery_shack_attic/mystery_shack_attic.glb"
#define doppio   "res/models/doppio/scene.gltf"
#define tr       "res/models/test_room/scene.glb"

/*
    Note: shinchan needs material 1
          fits, fits2 need material 0
          rest need debug_color 1
          (check ykr_model_loader.cpp)
*/

YK_API void _debug_app_start(struct YkDebugAppState* self)
{

    yk_renderer_innit(&self->ren, &self->window);

    
    size_t model_load_size = Gigabytes(1);
    YkMemoryArena scratch = yk_memory_sub_arena(&self->engine_memory.temp_storage, model_load_size);

    size_t model_load_temp = Megabytes(5);
    YkMemoryArena perm_sub = yk_memory_sub_arena(&self->engine_memory.perm_storage, model_load_temp);

    self->ren.test_meshes = ykr_load_mesh(&self->ren, fits, &scratch, &perm_sub, &self->ren.test_mesh_count);

    yk_memory_arena_zero(&self->engine_memory.temp_storage);
    ykr_load_mesh_cleanup();
}

YK_API void _debug_app_update(struct YkDebugAppState* self, f64 dt)
{
    yk_renderer_draw(&self->ren, &self->window, dt);
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
}

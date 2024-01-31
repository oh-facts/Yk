#include <yk_debug_app.h>

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
#define room     "res/models/room/scene.gltf"

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

    size_t one_c = 0;
    mesh_asset * one =    ykr_load_mesh(&self->ren, room, &scratch, &perm_sub, &one_c);
    ykr_load_mesh_cleanup();
    
    size_t two_c = 0;
    mesh_asset * two =   ykr_load_mesh(&self->ren, shinchan, &scratch, &perm_sub, &two_c);
    ykr_load_mesh_cleanup();

    size_t three_c = 0;
    mesh_asset* trhee = ykr_load_mesh(&self->ren, shinchan, &scratch, &perm_sub, &three_c);
    ykr_load_mesh_cleanup();
    
    yk_memory_arena_clean_reset(&self->engine_memory.temp_storage);

    for (u32 i = 0; i < two_c; i++)
    {
       //ToDo(facts): Store trans,rot and scale so you don't need to decompose model everytime you want to use it
        glm::vec3 scale, translation;
        glm::quat rotation;
        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose(two[i].model_mat, scale, rotation, translation, skew, perspective);

        glm::mat4 temp = glm::mat4(1);

        temp = glm::translate(temp, glm::vec3(-33, -31, -10.63));
        temp = glm::rotate(temp, 90 * DEG_TO_RAD, glm::vec3(0, 1, 0));
        temp = glm::scale(temp, scale);

        two[i].model_mat = temp ;
    }

    self->ren.test_meshes = one;
    self->ren.test_mesh_count = one_c + two_c + three_c;


    self->ren.cam.pos = glm::vec3{ -6.51f, -30.31f,-10.13f };
    self->ren.cam.yaw = -1.6f;

    yk_renderer_innit_scene(&self->ren);

    

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

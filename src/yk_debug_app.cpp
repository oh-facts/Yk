#include <yk_debug_app.h>
#include <math.h>
#include <yk.h>

/*
    I hope no one sees this. I don't want to be casted to the deep circles of hell
*/


YK_API void _debug_app_start(struct YkDebugAppState* self)
{
    yk_renderer_innit(&self->ren, &self->window);
    self->ren.test_meshes = yk_load_mesh(&self->ren, "res/models/shinchan/shinchan.gltf", self->engine_memory.temp_storage, Gigabytes(2), &self->ren.test_mesh_count);
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

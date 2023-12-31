#include <yk_debug_app.h>
#include <math.h>
#include <yk.h>


YK_API void _debug_app_start(struct YkDebugAppState* self)
{

    const vertex vertices[] = {
        {{-0.5f, -0.5f},{163 / 255.f, 163 / 255.f, 163 / 255.f} },
        {{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f} },
        {{0.5f, 0.5f}, {1.f, 1.f, 1.f}},
        {{-0.5f, 0.5f}, {128 / 255.f, 0.f, 128 / 255.f}}
    };

    const vertex vertices2[] = {
       {{-0.5f, -0.5f}, {1.0f, 33 / 255.0f, 140 / 255.0f}},
       {{0.5f, -0.5f}, {1.f, 216 / 255.f, 0}},
       {{0.5f, 0.5f}, {33 / 255.f, 177 / 255.f, 1.f}},
       {{-0.5f, 0.5f}, {1.0f, 33 / 255.0f, 140 / 255.0f}}
    };

    const u16 indices[] = {
        0, 1, 2, 2, 3, 0
    };

    yk_renderer_innit(&self->ren, &self->window);

    self->ro[0].id = 0;
    yk_renderer_innit_model(&self->ren, vertices, indices, &self->ro[0]);


    self->ro[1].id = 1;
    yk_renderer_innit_model(&self->ren, vertices2, indices, &self->ro[1]);


}

YK_API void _debug_app_update(struct YkDebugAppState* state)
{
    yk_renderer_draw_model(&state->ren, state->ro, 2, &state->window);

//	v3 a = { 1,3 };
//	printf("%f %f\n", a.x, a.y);
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

YK_API void _debug_app_shutdown(struct YkDebugAppState* state)
{
    yk_renderer_wait(&state->ren);
    yk_destroy_model(&state->ren, &state->ro[0]);
    yk_destroy_model(&state->ren, &state->ro[1]);

    yk_free_renderer(&state->ren);
}
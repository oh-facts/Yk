#include <yk_debug_app.h>
#include <math.h>
#include <yk.h>

/*
    I hope no one sees this. I don't want to be casted to the deep circles of hell
*/

int mode = 1;

void _start1(YkDebugAppState* self)
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

    render_object ro1 = {};
    ro1.id = 0;
    yk_renderer_innit_model(&self->ren, vertices, indices, &ro1);


    render_object ro2 = {};
    ro2.id = 1;
    yk_renderer_innit_model(&self->ren, vertices2, indices, &ro2);

    self->ren.num_ro = 2;
    self->ren.render_objects[0] = ro1;
    self->ren.render_objects[1] = ro2;
}

void _update1(YkDebugAppState* self)
{
    yk_renderer_raster_draw(&self->ren, &self->window);
}

void _sd1(YkDebugAppState* self)
{
    yk_destroy_model(&self->ren, &self->ren.render_objects[0]);
    yk_destroy_model(&self->ren, &self->ren.render_objects[1]);
}

void _start2(YkDebugAppState* self)
{
}


void _update2(YkDebugAppState* self)
{
    yk_renderer_draw(&self->ren, &self->window);
}


void _sd2(YkDebugAppState* self)
{
}


YK_API void _debug_app_start(struct YkDebugAppState* self)
{
    yk_renderer_innit(&self->ren, &self->window);

    switch (mode)
    {
    case 1:
    {
        _start1(self);
    }break;

    case 2:
    {
        _start2(self);
    }break;

    default:
        break;
    }

}

YK_API void _debug_app_update(struct YkDebugAppState* state)
{
    switch (mode)
    {
    case 1:
    {
        _update1(state);
    }break;

    case 2:
    {
        _update2(state);
    }break;

    default:
        break;
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

    switch (mode)
    {
    case 1:
    {
        _sd1(self);
    }break;

    case 2:
    {
        _sd2(self);
    }break;

    default:
        break;
    }
        
    yk_free_renderer(&self->ren);
}

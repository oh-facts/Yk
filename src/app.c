#include <app.h>
#include <math.h>
#include <yk.h>


void start(struct state* self)
{
    if (!self)
    {
        exit(-5);
    }
   

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

void update(struct state* state)
{
   
  if(!state->window.win_data.is_minimized)
    yk_renderer_draw_model(&state->ren, state->ro, 2, &state->window);

//	v3 a = { 1,3 };
//	printf("%f %f\n", a.x, a.y);
}

int is_running(struct state* self)
{
    return self->window.win_data.is_running;
}

void update_references(struct state* self)
{
   yk_recreate_swapchain(&self->ren, &self->window);
   // printf("%p", self->window.hinstance);
    //yk_renderer_innit(&self->ren, &self->window);
}
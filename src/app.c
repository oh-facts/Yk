#include <app.h>
#include <math.h>
#include <yk.h>

struct state
{
	YkWindow window;
    YkRenderer ren;
    render_object ro[2];
};


struct state* start()
{
    struct state* self = malloc(sizeof(struct state) * 2);
    memset(self, 0, sizeof(struct state));
    if (!self)
    {
        exit(-5);
    }
    yk_innit_window(&self->window);

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


    return self;
}

void update(struct state* state)
{
    yk_window_poll();
    yk_renderer_draw_model(&state->ren, state->ro, 2);

//	v3 a = { 1,3 };
//	printf("%f %f\n", a.x, a.y);
}

int is_running(struct state* self)
{
    return self->window.win_data.is_running;
}
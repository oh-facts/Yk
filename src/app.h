#ifndef YK_APP_H
#define YK_APP_H

#include <yk.h>
#include <yk_api.h>

struct state
{
    YkWindow window;
    YkRenderer ren;
    render_object ro[2];
};


YK_API void start(struct state* state);
YK_API void update(struct state* state);
YK_API void freeApp(struct state* state);

YK_API int is_running(struct state* state);

YK_API void update_references(struct state* state);
#endif // !YK_APP_H
#ifndef YK_APP_H
#define YK_APP_H

#include <yk.h>
#include <yk_api.h>

struct state;

YK_API struct state* start();
YK_API void update(struct state* state);

YK_API int is_running(struct state* state);

#endif // !YK_APP_H
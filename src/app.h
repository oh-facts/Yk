#ifndef YK_APP
#define YK_APP

#include <yk.h>
#include <yk_api.h>

typedef void (*Update)(YkRenderer* ren, YkWindow* win, render_object* ros);


YK_API void update(YkRenderer* ren, YkWindow* win, render_object* ros);

#endif // !YK_APP

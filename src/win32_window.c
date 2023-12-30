#include <win32_window.h>


void yk_free_window(YkWindow* window)
{
    DestroyWindow(window->win_handle);
}


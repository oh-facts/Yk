#ifndef YK_WINDOW_H
#define YK_WINDOW_H

#include<yk.h>
#include <yk_api.h>

/*
	ToDo(facts): Platform layer + make these opaque
*/

typedef struct win_data
{
	//ToDo(facts) 12/31 0548: Make these flags a bitmask 
	b8 is_resized;
	b8 is_minimized;
	b8 is_running;
	i32 size_x;
	i32 size_y;
}win_data;

struct YkWindow
{
	//test is a debug key press I use, until I get input properly implemented
	int test;
	void * win_handle;
	win_data win_data;
};

YK_API void yk_innit_window(YkWindow* window);
YK_API void yk_window_poll();
YK_API void yk_free_window(YkWindow* window);
YK_API void yk_get_framebuffer_size(YkWindow* win, u32* width, u32* height);


#endif // !YK_WIN32_WINDOW_H

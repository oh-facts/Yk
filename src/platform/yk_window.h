#ifndef YK_WINDOW_H
#define YK_WINDOW_H

#include<yk.h>
#include <yk_api.h>
#include <platform/yk_input.h>
typedef struct YkWindow YkWindow;

typedef struct win_data
{
	//ToDo(facts) 12/31 0548: Make these flags a bitmask 
	//			  1/17: I hate this struct I made. I hate it I hate it. 
	//					I should have just left this stuff entirely inside YkWindow.
	//					I don't remember why I separated it.
	b8 is_resized;
	b8 is_minimized;
	b8 is_running;
	i32 size_x;
	i32 size_y;
}win_data;

struct YkWindow
{
	void * win_handle;
	win_data win_data;
	YkKeyState keys;
	YkMouseClickState clicks;
	YkMousePosState mouse_pos;
};

YK_API void yk_innit_window(YkWindow* window);
YK_API void yk_window_poll();
YK_API void yk_window_update(YkWindow* window);
YK_API void yk_free_window(YkWindow* window);

//ToDo(facts): Figure out how to have this function's definition to exist without letting renderer.cpp link with it. For now I am just putting this function inside renderer.cpp
// Since renderer needs to include windows.h anyways to work properly. But this is a window function and I want to be able to have this here, in the platform layer. But the platform
// layer shouldn't link with the game layer. 
// Also I realized this is happening mainly because renderer is platform state and once I have a game layer this wont be a problem. But I'm sure there's a better solution
//YK_API void yk_get_framebuffer_size(YkWindow* win, u32* width, u32* height);

#endif // !YK_WIN32_WINDOW_H

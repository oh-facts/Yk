#ifndef YK_WIN32_WINDOW_H
#define YK_WIN32_WINDOW_H

#include<yk.h>
#include <Windows.h>
#include <yk_api.h>

typedef struct YkWindow YkWindow;

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
	HWND win_handle;
	HINSTANCE hinstance;
	win_data win_data;
};

YK_API LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
YK_API void yk_innit_window(YkWindow* window);
YK_API void yk_window_poll();
YK_API void yk_free_window(YkWindow* window);

typedef void* YkTime;

YK_API void yk_time_innit(YkTime* time);
YK_API f32 yk_get_time(YkTime* time);


#endif // !YK_WIN32_WINDOW_H

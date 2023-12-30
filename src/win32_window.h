#ifndef YK_WIN32_WINDOW_H
#define YK_WIN32_WINDOW_H

#include<yk.h>
#include <Windows.h>
#include <yk_api.h>

typedef struct YkWindow YkWindow;

typedef struct win_data
{
	b8 is_resized;
	b8 is_minimized;
	b8 is_running;
	i32 size_x;
	i32 size_y;
}win_data;

typedef win_data(*resize_callback)(void);

struct YkWindow
{
	HWND win_handle;
	HINSTANCE hinstance;
	win_data win_data;
};
void yk_free_window(YkWindow* window);

YK_API LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
YK_API void yk_innit_window(YkWindow* window);
YK_API void yk_window_poll();

#endif // !YK_WIN32_WINDOW_H

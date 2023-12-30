#ifndef YK_WIN32_WINDOW_H
#define YK_WIN32_WINDOW_H

#include<yk.h>
#include <Windows.h>


typedef struct YkWindow YkWindow;

extern YkWindow window;

typedef struct win_data
{
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

void yk_innit_window(YkWindow* window);
void yk_window_poll();
void yk_free_window(YkWindow* window);



#endif // !YK_WIN32_WINDOW_H

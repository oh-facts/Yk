#ifndef YK_WIN32_WINDOW_H
#define YK_WIN32_WINDOW_H

#include<yk.h>
#include <Windows.h>

#define WIN_SIZE_X 800
#define WIN_SIZE_Y 600

typedef struct YkWindow YkWindow;
typedef b8(*win_closed)(void);

struct YkWindow
{
	HWND win_handle;
	HINSTANCE hinstance;
	b8 is_minimized;
	b8 is_running;

};

void yk_innit_window(YkWindow* window);
void yk_window_poll();
void yk_free_window(YkWindow* window);



#endif // !YK_WIN32_WINDOW_H

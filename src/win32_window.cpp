#include <yk_window.h>
#include <Windows.h>

#define WIN_SIZE_X 800
#define WIN_SIZE_Y 600

YK_API LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
    case WM_KEYDOWN:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        switch (wParam) {
        case VK_ESCAPE:
        {
            win->test = 1;
        }break;
        }
    }break;

    case WM_CREATE:
    {
        YkWindow* win = (YkWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)win);
    }

    break;
    case WM_SIZE:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        win->win_data.size_x = LOWORD(lParam);
        win->win_data.size_y = HIWORD(lParam);
        win->win_data.is_resized = true;
        if (wParam == SIZE_MINIMIZED)
        {
            if (win)
                win->win_data.is_minimized = true;
        }
        else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
        {
            if (win)
                win->win_data.is_minimized = false;
        }
    } break;
    case WM_DESTROY:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (win)
            win->win_data.is_running = false;
        PostQuitMessage(0);
    }

    break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


void yk_innit_window(YkWindow* window)
{
    window->win_data.is_running = true;
    window->win_data.is_minimized = false;
    window->win_data.size_x = WIN_SIZE_X;
    window->win_data.size_y = WIN_SIZE_Y;
    window->test = 0;

    WNDCLASS wc = { };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(0);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "MainWindowClass";

    if (!RegisterClass(&wc))
        exit(-1);

    window->win_handle = CreateWindowA(wc.lpszClassName, "yekate", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WIN_SIZE_X, WIN_SIZE_Y,
        NULL, NULL, wc.hInstance, window);

    if (!window->win_handle)
    {
        printf("Win32 window Creation failure");
        exit(-1);
    }

    ShowWindow((HWND)window->win_handle, SW_SHOWNORMAL);
    UpdateWindow((HWND)window->win_handle);
}

void yk_window_poll()
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}


void yk_free_window(YkWindow* window)
{
    DestroyWindow((HWND)window->win_handle);
}

/*
void yk_get_framebuffer_size(YkWindow* win, u32* width, u32* height)
{
    RECT clientRect;
    GetClientRect((HWND)win->win_handle, &clientRect);

    *width = (u32)clientRect.right - clientRect.left;
    *height = (u32)clientRect.bottom - clientRect.top;
}
*/
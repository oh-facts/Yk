#include <win32_window.h>

b8 is_minimized;
b8 is_closed;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            is_minimized = true;
        }
        else if (wParam == SIZE_RESTORED)
        {
            is_minimized = false;
        }

        break;

    case WM_DESTROY:
        is_closed = true;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


void yk_innit_window(YkWindow* window)
{   
    is_minimized = false;
    is_closed = false;
    window->is_minimized = false;
    window->hinstance = GetModuleHandle(0);
 
    WNDCLASS wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = window->hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "MainWindowClass";

    if (!RegisterClass(&wc))
        exit(-1);

    window->win_handle = CreateWindowA(wc.lpszClassName, "yekate", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WIN_SIZE_X, WIN_SIZE_Y,
        NULL, NULL, wc.hInstance, NULL);

    if (!window->win_handle)
    {
        printf("Win32 window Creation failure");
        exit(-1);
    }
        

    ShowWindow(window->win_handle, SW_SHOWNORMAL);
    UpdateWindow(window->win_handle);
}

void yk_free_window(YkWindow* window)
{
    DestroyWindow(window->win_handle);
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
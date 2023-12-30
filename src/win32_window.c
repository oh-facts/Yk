#include <win32_window.h>

#define WIN_SIZE_X 800
#define WIN_SIZE_Y 600

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    YkWindow* window = (YkWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_CREATE:
        window = (YkWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
        window->win_data.is_running = true;
        window->win_data.is_minimized = false;
        break;

    case WM_DESTROY:
        window->win_data.is_running = false;
        free(window);
        break;

    case WM_SIZE:
        window->win_data.size_x = LOWORD(lParam);
        window->win_data.size_y = HIWORD(lParam);
        window->win_data.is_minimized = wParam == SIZE_MINIMIZED;
        break;

    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE)
        {
            window->win_data.is_minimized = true;
        }
        else if ((wParam & 0xFFF0) == SC_RESTORE)
        {
            window->win_data.is_minimized = false;
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

YkWindow yk_innit_window()
{
    YkWindow* window = malloc(sizeof(YkWindow));
    window->win_data.is_running = true;
    window->win_data.is_minimized = false;
    window->win_data.size_x = WIN_SIZE_X;
    window->win_data.size_y = WIN_SIZE_Y;

    WNDCLASS wc = { 0 };
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

    SetWindowLongPtr(window->win_handle, GWLP_USERDATA, (LONG_PTR)window);

    ShowWindow(window->win_handle, SW_SHOWNORMAL);
    UpdateWindow(window->win_handle);

    return *window;
}
void yk_free_window(YkWindow* window)
{
    DestroyWindow(window->win_handle);
}

void yk_window_poll(YkWindow* window)
{
    if (window == 0)
    {
        printf("rerere");
    }
    MSG message;
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

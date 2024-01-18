#include <platform/yk_window.h>
#include <Windows.h>
#include <windowsx.h>
#include <platform/yk_input.h>

#define WIN_SIZE_X 800
#define WIN_SIZE_Y 600

YK_API LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
    //https://blog.molecular-matters.com/2011/09/05/properly-handling-keyboard-input/
    case WM_INPUT:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

        char buffer[sizeof(RAWINPUT)] = {};
        UINT size = sizeof(RAWINPUT);

        GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
       
        RAWINPUT* raw = (RAWINPUT*)(buffer);
        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {

            const RAWKEYBOARD* rawKB = &raw->data.keyboard;

            UINT virtualKey = rawKB->VKey;
            UINT scanCode = rawKB->MakeCode;
            UINT flags = rawKB->Flags;

            if (virtualKey == 255)
            {
                // discard "fake keys" which are part of an escaped sequence
                return 0;
            }
            else if (virtualKey == VK_SHIFT)
            {
                // correct left-hand / right-hand SHIFT
                virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
            }
            else if (virtualKey == VK_NUMLOCK)
            {
                // correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
                scanCode = (MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | 0x100);
            }

            const bool isE0 = ((flags & RI_KEY_E0) != 0);
            const bool isE1 = ((flags & RI_KEY_E1) != 0);

            if (isE1)
            {
                // for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
                // however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
                if (virtualKey == VK_PAUSE)
                    scanCode = 0x45;
                else
                    scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
            }

            switch (virtualKey)
            {
                // right-hand CONTROL and ALT have their e0 bit set
            case VK_CONTROL:
                if (isE0)
                    virtualKey = VK_LCONTROL;
                else
                    virtualKey = VK_RCONTROL;
                break;
            case VK_MENU:
                if (isE0)
                    virtualKey = VK_RMENU;
                else
                    virtualKey = VK_LMENU;
                break;

                // NUMPAD ENTER has its e0 bit set
                /*
                * I don't care for numpad enter
            case VK_RETURN:
                if (isE0)
                    virtualKey = VK_NUMPAD_ENTER (FUCK MICROSOFT!);
                break;
                */
                // the standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their e0 bit set, but the
                // corresponding keys on the NUMPAD will not.
            case VK_INSERT:
                if (!isE0)
                    virtualKey = VK_NUMPAD0;
                break;
            /*
            case VK_DELETE:
                if (!isE0)
                    virtualKey = VK_NUMPADP_DECIMAL// (FUCK MICROSOFT!);
                break;
            */
            case VK_HOME:
                if (!isE0)
                    virtualKey = VK_NUMPAD7;
                break;

            case VK_END:
                if (!isE0)
                    virtualKey = VK_NUMPAD1;
                break;

            case VK_PRIOR:
                if (!isE0)
                    virtualKey = VK_NUMPAD9;
                break;

            case VK_NEXT:
                if (!isE0)
                    virtualKey = VK_NUMPAD3;
                break;

                // the standard arrow keys will always have their e0 bit set, but the
                // corresponding keys on the NUMPAD will not.
            case VK_LEFT:
                if (!isE0)
                    virtualKey = VK_NUMPAD4;
                break;

            case VK_RIGHT:
                if (!isE0)
                    virtualKey = VK_NUMPAD6;
                break;

            case VK_UP:
                if (!isE0)
                    virtualKey = VK_NUMPAD8;
                break;

            case VK_DOWN:
                if (!isE0)
                    virtualKey = VK_NUMPAD2;
                break;

                // NUMPAD 5 doesn't have its e0 bit set
            case VK_CLEAR:
                if (!isE0)
                    virtualKey = VK_NUMPAD5;
                break;
            }

            const bool wasUp = ((flags & RI_KEY_BREAK) != 0);

            win->keys._cur[virtualKey] = !wasUp;

            // getting a human-readable string

#if 0
            UINT key = (scanCode << 16) | (isE0 << 24);
            char buffer[512] = {};
            GetKeyNameText((LONG)key, buffer, 512);
            
            printf("%d %s %d\n", virtualKey, buffer, wasUp);
#endif
        }
        else if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN) 
            {
                win->clicks._cur[YK_MOUSE_BUTTON_LEFT] = 1;
            }
            else if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_UP) 
            {
                win->clicks._cur[YK_MOUSE_BUTTON_LEFT] = 0;
            }

            if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN)
            {
                win->clicks._cur[YK_MOUSE_BUTTON_RIGHT] = 1;
            }
            else if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_UP)
            {
                win->clicks._cur[YK_MOUSE_BUTTON_RIGHT] = 0;
            }


        }

    }break;
    case WM_MOUSEMOVE:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        i32 x_pos = GET_X_LPARAM(lParam);
        i32 y_pos = GET_Y_LPARAM(lParam);

        win->mouse_pos.cur.x = yk_clamp(x_pos, 0, win->win_data.size_x);
        win->mouse_pos.cur.y = yk_clamp(y_pos, 0, win->win_data.size_y);;

        //ykm_print_v2(yk_input_mouse_mv(&win->mouse_pos));
    }break;
    /*
    * Currently discarded these events from shooting entirely since I don't need them. Will be relevant when I start with UI
    case WM_KEYDOWN:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        switch (wParam) 
        {
            case VK_ESCAPE:
            {  
                win->test = 1;
            }break;
            
            case 0x41:
            {
                printf("A pressed");
            }

        }
    }break;
    */
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
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


void yk_innit_window(YkWindow* window)
{
    window->win_data.is_running = true;
    window->win_data.is_minimized = false;
    window->win_data.size_x = WIN_SIZE_X;
    window->win_data.size_y = WIN_SIZE_Y;
   
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

    /*
        0 is keyboard
        1 is mouse
    */
    RAWINPUTDEVICE devices[2] = {};

    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x06;
    devices[0].dwFlags = 0;
    devices[0].hwndTarget = (HWND)window->win_handle;

    devices[1].usUsagePage = 0x01;
    devices[1].usUsage = 0x02;
    devices[1].dwFlags = 0;
    devices[1].hwndTarget = (HWND)window->win_handle;

    RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE));
  

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

void yk_window_update(YkWindow* window)
{
    for (u32 i = 0; i < 256; i++)
    {
        window->keys._old[i] = window->keys._cur[i];
    }

    for (u32 i = 0; i < 3; i++)
    {
        window->clicks._old[i] = window->clicks._cur[i];
    }

    window->mouse_pos.old = window->mouse_pos.cur;

    /*
    POINT point;
    if (GetCursorPos(&point)) 
    {
        if (ScreenToClient((HWND)window->win_handle, &point)) 
        {
           window->mouse_pos.x = point.x;
           window->mouse_pos.y = point.y;

           if (window->mouse_pos.x < 0)
               window->mouse_pos.x = 0;
           if (window->mouse_pos.y < 0)
               window->mouse_pos.y = 0;
        }
    }
    */
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

b8 yk_input_is_key_tapped(YkKeyState* state, u32 key)
{
    return state->_cur[key] && !state->_old[key];   
}

b8 yk_input_is_key_held(YkKeyState* state, u32 key)
{
    return state->_cur[key];
}

b8 yk_input_is_key_released(YkKeyState* state, u32 key)
{
    return !state->_cur[key] && state->_old[key];
}

b8 yk_input_is_click(YkMouseClickState* state, YK_MOUSE_BUTTON button)
{
    return state->_cur[button] && !state->_old[button];
}

v2 yk_input_mouse_mv(YkMousePosState* state)
{
    //signs reversed for y because I want +y to be up and -y to be down, and by default, the origin of
    //a window is top left corner (so window is in the "4th quadrant", but I want "1st quadrant" behaviour)
    return v2{ state->cur.x - state->old.x , - state->cur.y + state->old.y};
}
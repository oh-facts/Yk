#include "yk_input.h"
#include <Windows.h>

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
    return v2{ state->rel.x, -state->rel.y };
}

void yk_show_cursor(b8 flag)
{
    if (flag == false)
    {
        while (ShowCursor(false) >= 0);
    }
    else
    {
        ShowCursor(true);
    }
    
}

void yk_clip_cusor(void* win_handle, b8 flag)
{
    if (flag == false)
    {
        ClipCursor(0);
    }
    else
    {
        RECT rect;
        GetClientRect((HWND)win_handle, &rect);

        ClientToScreen((HWND)win_handle, (POINT*)&rect.left);
        ClientToScreen((HWND)win_handle, (POINT*)&rect.right);
        ClipCursor(&rect);
    }
}
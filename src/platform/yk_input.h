#ifndef YK_INPUT_H
#define YK_INPUT_H

#include <defines.h>
#include <yk_api.h>
#include <yk_math.h>

enum YK_KEY
{
	/*
		ignore
	*/
	YK_KEY_LSHIFT = 0xA0,
	YK_KEY_LCTRL = 0xA2,
};

struct YkKeyState
{
	b8 _cur[256];
	b8 _old[256];
};

struct YkMouseClickState
{
	//0 - left, 1 - right, 2 - middle
	b8 _cur[3];
	b8 _old[3];
};

enum YK_MOUSE_BUTTON
{
	YK_MOUSE_BUTTON_LEFT,
	YK_MOUSE_BUTTON_RIGHT,
	YK_MOUSE_BUTTON_MIDDLE,
};

struct YkMousePosState
{
	/*
		cur and old don't serve purpose anymore
		since  rel is  them but better. It will 
		remain  however,  since it is  part  of 
		family  now  ( I'm joking, it has minor 
		use)
	*/
	v2 cur;
	v2 old;
	v2 rel;
};

b8 yk_input_is_key_tapped(YkKeyState* state, u32 key);
b8 yk_input_is_key_held(YkKeyState* state, u32 key);
b8 yk_input_is_key_released(YkKeyState* state, u32 key);
b8 yk_input_is_click(YkMouseClickState* state, YK_MOUSE_BUTTON button);
v2 yk_input_mouse_mv(YkMousePosState* state);
void yk_show_cursor(b8 flag);
void yk_clip_cusor(void* win_handle, b8 flag);
#endif // !YK_INPUT_H

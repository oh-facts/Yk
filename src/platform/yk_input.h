#ifndef YK_INPUT_H
#define YK_INPUT_H

enum YK_KEY
{
	/*
		ignore
	*/
	YK_KEY_A,
	YK_KEY_B,
	YK_KEY_C,
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
	v2 cur;
	v2 old;
};

b8 yk_input_is_key_tapped(YkKeyState* state, u32 key);
b8 yk_input_is_key_held(YkKeyState* state, u32 key);
b8 yk_input_is_key_released(YkKeyState* state, u32 key);

b8 yk_input_is_click(YkMouseClickState* state, YK_MOUSE_BUTTON button);
v2 yk_input_mouse_mv(YkMousePosState* state);

#endif // !YK_INPUT_H
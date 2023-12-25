#ifndef YK_MATH
#define YK_MATH

#include <yk.h>


union v2 {
	struct 
	{
		f32 x, y;
	};
	f32 e[2];
};

union v3 {
	struct
	{
		f32 x, y, z;
	};
	f32 e[3];
};

union v4 {
	struct
	{
		f32 x, y, z, w;
	};
	f32 e[4];
};

typedef union v2 v2;
typedef union v3 v3;
typedef union v4 v4;

inline v4 yk_v4_add(v4 a, v4 b)
{
	v4 out = { 0 };
	out.x = a.x + b.x;
	out.y = a.y + b.y;
	out.z = a.z + b.z;
	out.w = a.w + b.w;
	
	return out;
}

inline v4 yk_v4_sub(v4 a, v4 b)
{
	v4 out = { 0 };
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
	out.w = a.w - b.w;

	return out;
}

#endif // !YK_MATH

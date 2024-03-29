#ifndef YK_MATH
#define YK_MATH

/*
	This will likely be retired.
	At this point I only use the
	types  for convenience.  For
	v2 instead of f32[2] because
	v2 gives me QoL features.
	For actual math, use the glm
	library.
*/

#include <math.h>
#include <defines.h>
#define PI 	(3.14159265359f)
#define DEG_TO_RAD (PI/180.f)
#define RAD_TO_DEG (180.f/PI)
#include <iostream>
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

/*
	e[row][column]
*/
struct m4x4
{
	f32 e[4][4];
};

typedef union v2 v2;
typedef union v3 v3;
typedef union v4 v4;

typedef struct m4x4 m4;

inline f32 yk_v3_length(const v3 a)
{
	f32 out;
	out = sqrtf((a.x) * (a.x) + (a.y) * (a.y) + (a.z) * (a.z));
	return out;
}

inline v3 yk_v3_normalize(const v3 a)
{
	v3 out = { };

	f32 l = yk_v3_length(a);
	out.x = a.x / l;
	out.y = a.y / l;
	out.z = a.z / l;
	
	return out;
}

inline f32 yk_v3_dot(const v3 a, const v3 b)
{
	f32 out;
	out = a.x * b.x + a.y * b.y + a.z * b.z;
	return out;
}

inline v3 yk_v3_cross(const v3 a, const v3 b)
{
	v3 out = { };
	
	out.x = a.y * b.z - a.z * b.y;
	out.y = a.z * b.x - a.x * b.z;
	out.z = a.x * b.y - a.y * b.x;

	return out;
}

inline v3 yk_v3_sub(const v3 a, const v3 b)
{
	v3 out = { };
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
	return out;
}

inline v4 yk_v4_add(v4 a, v4 b)
{
	v4 out = { };
	out.x = a.x + b.x;
	out.y = a.y + b.y;
	out.z = a.z + b.z;
	out.w = a.w + b.w;
	
	return out;
}

inline v4 yk_v4_sub(v4 a, v4 b)
{
	v4 out = { };
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
	out.w = a.w - b.w;

	return out;
}

inline v4 yk_v4_mul(v4 a, v4 b)
{
	v4 out = {};
	out.x = a.x * b.x;
	out.y = a.y * b.y;
	out.z = a.z * b.z;
	out.w = a.w * b.w;

	return out;
}

inline m4 yk_m4_identity()
{
	m4 out =
	{
		1., 0., 0., 0.,
		0., 1., 0., 0.,
		0., 0., 1., 0.,
		0., 0., 0., 1.,
	};

	return out;
}

inline m4 yk_m4_translate(const m4 mat, const v3 vec)
{
	m4 out = mat;

	out.e[3][0] += vec.x;
	out.e[3][1] += vec.y;
	out.e[3][2] += vec.z;
	//out.e[3][3] = out.e[0][3] * vec.x + mat.e[1][3] * vec.y + mat.e[2][3] * vec.z;

	return out;
}

inline m4 yk_m4_rotate(const m4 mat, const f32 angle, const v3 axis)
{
	f32 c = cosf(angle);
	f32 s = sinf(angle);
	f32 t = 1.f - c;

	v3 _axis = yk_v3_normalize(axis);
	f32 x = _axis.x;
	f32 y = _axis.y;
	f32 z = _axis.z;

	m4 out = mat;

	out.e[0][0] = t * x * x + c;
	out.e[0][1] = t * x * y - s * z;
	out.e[0][2] = t * x * z + s * y;

	out.e[1][0] = t * x * y + s * z;
	out.e[1][1] = t * y * y + c;
	out.e[1][2] = t * y * z - s * x;

	out.e[2][0] = t * x * z - s * y;
	out.e[2][1] = t * y * z + s * x;
	out.e[2][2] = t * z * z + c;

	//out.e[3][3] = 1.0;

	return out;
}

inline m4 yk_m4_scale(const m4 mat, const v3 scale)
{
	m4 out = mat;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			out.e[i][j] *= scale.e[i];
		}
	}

	return out;
}

inline m4 yk_m4_look_at(const v3 eye, const v3 target, const v3 up)
{
	v3 zAxis = yk_v3_normalize(yk_v3_sub(target, eye));

	v3 xAxis = yk_v3_normalize(yk_v3_cross(zAxis, up));

	v3 yAxis = yk_v3_cross(xAxis, zAxis);

	m4 out = { };

	out.e[0][0] = xAxis.x;
	out.e[0][1] = yAxis.x;
	out.e[0][2] = -(zAxis.x);
	out.e[0][3] = 0.0f;

	out.e[1][0] = xAxis.y;
	out.e[1][1] = yAxis.y;
	out.e[1][2] = -(zAxis.y);
	out.e[1][3] = 0.0f;

	out.e[2][0] = xAxis.z;
	out.e[2][1] = yAxis.z;
	out.e[2][2] = -(zAxis.z);
	out.e[2][3] = 0.0f;

	out.e[3][0] =  -yk_v3_dot(xAxis, eye);
	out.e[3][1] = -yk_v3_dot(yAxis, eye);
	out.e[3][2] = yk_v3_dot(zAxis, eye);
	out.e[3][3] = 1.0f;

	return out;

}

inline m4 yk_m4_perspective(const f32 fov_degrees, const f32 aspect_ratio, const f32 near_, const f32 far_) //near_ far_ -> fuck you microsoft! (John Malkovitch voice)
{
	m4 out = { };
	f32 f = 1 / tan(fov_degrees / 2);

	out.e[0][0] = f / aspect_ratio;
	out.e[1][1] = f;
	out.e[2][2] = (far_ + near_) / (near_ - far_);
	out.e[2][3] = -1.0;
	out.e[3][2] = 2.0 * far_ * near_ / (near_ - far_*1.f);

	return out;


}

inline m4 yk_m4_multiply(const m4 a, const m4 b)
{
	m4 out = { 0 };

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			for (int k = 0; k < 4; ++k)
			{
				out.e[i][j] += a.e[i][k] * b.e[k][j];
			}
		}
	}

	return out;
}

inline int yk_clamp(int value, int min, int max) {
	return value < min ? min : (value > max ? max : value);
}

inline void ykm_print_v3(v3 v3)
{
	printf("[ %f , %f , %f ]\n", v3.x, v3.y, v3.z);
}

inline void ykm_print_v2(v2 v2)
{
	printf("[ %f , %f]\n", v2.x, v2.y);
}

#endif // !YK_MATH

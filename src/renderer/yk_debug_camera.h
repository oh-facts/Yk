#ifndef YK_DEBUG_CAMERA_H
#define YK_DEBUG_CAMERA_H

#include <defines.h>

struct YkDebugCamera
{
	/*
		Input data
		_rot.x = yaw
		_rot.y = pitch
	*/
	glm::vec3 _vel;
	glm::vec2 _rot;

	glm::vec3 pos;
	f32 yaw;
	f32 pitch;
};

glm::mat4 ykr_camera_get_view_matrix(YkDebugCamera* self);
glm::mat4 ykr_camera_get_rot_matrix(YkDebugCamera* self);
void ykr_camera_update(YkDebugCamera* self, f32 delta);
void ykr_camera_input(YkDebugCamera* self, struct YkWindow* window);

#endif // ! YK_DEBUG_CAMERA_H

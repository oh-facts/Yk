#ifndef YK_DEBUG_CAMERA_H
#define YK_DEBUG_CAMERA_H
#include <glm/glm.hpp>
#include <defines.h>


#define cam_speed 1.f

struct YkDebugCamera
{
	glm::vec3 _vel;
	glm::vec3 pos;
	f32 pitch;
	f32 yaw;
};

glm::mat4 ykr_camera_get_view_matrix(YkDebugCamera* self);
glm::mat4 ykr_camera_get_rot_matrix(YkDebugCamera* self);
void ykr_camera_update(YkDebugCamera* self, f32 delta);
void ykr_camera_input(YkDebugCamera* self, struct YkWindow* window);

#endif // ! YK_DEBUG_CAMERA_H

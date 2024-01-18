#include <renderer/yk_debug_camera.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <platform/yk_window.h>

glm::mat4 ykr_camera_get_view_matrix(YkDebugCamera* self)
{
	glm::mat4 cam_trans = glm::translate(glm::mat4(1.f), self->pos);
	glm::mat4 cam_rot = ykr_camera_get_rot_matrix(self);

	return glm::inverse(cam_trans * cam_rot);
}

glm::mat4 ykr_camera_get_rot_matrix(YkDebugCamera* self)
{
	glm::quat pitch_rot = glm::angleAxis(self->pitch, glm::vec3{ 1.f, 0.f, 0.f });
	glm::quat yaw_rot = glm::angleAxis(self->yaw, glm::vec3{ 0.f, -1.f, 0.f });

	return glm::toMat4(yaw_rot) * glm::toMat4(pitch_rot);
}

void ykr_camera_update(YkDebugCamera* self, f32 delta)
{
	glm::mat4 cam_rot = ykr_camera_get_rot_matrix(self);
	self->pos += glm::vec3(cam_rot * glm::vec4(self->_vel * cam_speed * delta, 0.f));

	self->yaw += self->_rot.x * delta * cam_sens;
	self->pitch += self->_rot.y * delta * cam_sens;

	self->_vel = glm::vec3(0.f);
}

void ykr_camera_input(YkDebugCamera* self, struct YkWindow* window)
{
	
	if (yk_input_is_key_held(&window->keys, 'W'))
	{
		self->_vel.z = -1;
		//printf("w");
	}
	if (yk_input_is_key_held(&window->keys, 'A'))
	{
		self->_vel.x = -1;
		//printf("a");
	}
	if (yk_input_is_key_held(&window->keys, 'S'))
	{
		self->_vel.z = 1;
		//printf("s");
	}
	if (yk_input_is_key_held(&window->keys, 'D'))
	{
		self->_vel.x = 1;
		//printf("d");
	}

	if (yk_input_is_key_held(&window->keys, YK_KEY_LSHIFT))
	{
		self->_vel.y = 1;
	}

	if (yk_input_is_key_held(&window->keys, YK_KEY_LCTRL))
	{
		self->_vel.y = -1;
	}

	self->_rot.x = (f32)yk_input_mouse_mv(&window->mouse_pos).x;
	self->_rot.y = (f32)yk_input_mouse_mv(&window->mouse_pos).y;
}
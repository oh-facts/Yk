#ifndef YK_RENDERER_DEVICE_H
#define YK_RENDERER_DEVICE_H

#include <renderer/ella.h>
#include <renderer/common.h>
#include <renderer/context.h>

typedef struct mn_context mn_context;
typedef struct mn_device mn_device;

struct mn_device
{
	VkDevice handle;
	VkPhysicalDevice phys_device;
	VkQueue gfx_q;
	i32 qfams[3];

	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffers[MAX_FRAMES_IN_FLIGHT];

	VkSemaphore image_available_semawhores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semawhores[MAX_FRAMES_IN_FLIGHT];
	VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

	uint32_t current_frame;

	VkBuffer vert_buffer;
	VkDeviceMemory vert_buffer_memory;

	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;

	VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
	void* uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

};

void mn_device_innit(mn_context* context, VkSurfaceKHR surface, mn_device* device);

#endif // !YK_RENDERER_DEVICE_H

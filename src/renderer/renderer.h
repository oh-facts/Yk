#ifndef RENDERER_H
#define RENDERER_H

#include "context.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
/*
	renderer. Internally, the  renderer is called ella.
	Arbitrary easy to write name. It is to avoid having
	to write  yk_renderer_device_create and just  write
	ella_device_create(). This  is only internal naming
	and no  one interacting with the renderer front end
	should bother
*/

typedef struct yk_renderer yk_renderer;
typedef struct window_data window_data;
struct yk_renderer
{
	struct mn_context context;
	struct mn_device device;
	VkSurfaceKHR surface;
	struct mn_swapchain swapchain;
	struct mn_pipeline pipeline;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSets[2];

	// 
	VkBuffer vert_buffer;
	VkDeviceMemory vert_buffer_memory;

	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;

	VkBuffer uniformBuffers[2];
	VkDeviceMemory uniformBuffersMemory[2];
	void* uniformBuffersMapped[2];

	//
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffers[MAX_FRAMES_IN_FLIGHT];

	VkSemaphore image_available_semawhores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semawhores[MAX_FRAMES_IN_FLIGHT];
	VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

	uint32_t current_frame;

	window_data* win_data;

};

struct window_data
{
	b8 is_running;
	b8 is_minimized;
	f32 x, y;
	void* win_handle;
};

void yk_renderer_innit(yk_renderer* self, window_data * window_data);

void yk_renderer_draw(yk_renderer* self);


enum Q_FAM
{
	Q_FAM_GFX,
	Q_FAM_GFX_COMPUTE,
	Q_FAM_PRESENT,
	Q_FAM_SIZE
};


#endif // !RENDERER_H

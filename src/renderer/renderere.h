#ifndef YK_RENDERERE_H
#define YK_RENDERERE_H

#include <renderer/common.h>
#include <yk.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <yk_math.h>

#define max_images 3
#define MAX_FRAMES_IN_FLIGHT 2

typedef struct YkRenderere YkRenderere;

/*
* Most of this is internal state and unrequired by other structs. Still, I don't know enough about vulkan to want to abstract this away.
* So this will be a megastruct until then.
*/
struct YkRenderere
{
	VkInstance vk_instance;
	//ToDo(facts 12/24 0341): Does renderer receive a window handle? Or does the renderer own the window? I want to be able to support multiple windows
	//for whatever reason. I will get back to this later. For now, a window handle should be fine.
	struct YkWindow* window_handle;
	VkSurfaceKHR surface;
	VkPhysicalDevice phys_device;
	VkDevice device;
	VkQueue gfx_q;
	i32 qfams[3];
	VkSwapchainKHR swapchain;

	VkViewport viewport;
	VkRect2D scissor;
	VkExtent2D extent;
	
	VkImage swapchain_image_list[max_images];
	VkImageView swapchain_image_view_list[max_images];

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipeline_layout;
	VkPipeline gfx_pipeline;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSets[2];

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


#if VK_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

void yk_innit_renderer(YkRenderere* renderer, struct YkWindow* window);

void yk_free_renderer(YkRenderere* renderer);

void vk_draw_frame(YkRenderere* renderer);

void yk_renderer_wait(YkRenderere* renderer);


struct vertex
{
	v2 pos;
	v3 color;
};


VkVertexInputBindingDescription vk_get_binding_desc();
void get_attrib_desc(VkVertexInputAttributeDescription out[]);

struct ubo
{
	m4 model;
	m4 view;
	m4 proj;
};

typedef struct ubo ubo;
#endif
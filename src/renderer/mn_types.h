#ifndef RENDERER_MN_TYPES
#define RENDERER_MN_TYPES

#include "../vendor/vma/include/vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <yk_math.h>
#include <yk_math.h>

#include <yk_api.h>
#include <time.h>

#define VkDEBUG 1

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 0


typedef struct vertex vertex;

#define max_images 3

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct vertex
{
	v2 pos;
	v3 color;
}vertex;


typedef struct mvp_matrix
{
	m4 model;
	m4 view;
	m4 proj;
}mvp_matrix;

typedef struct buffer
{
	VkBuffer handle;
	VkDeviceMemory memory;
}buffer;

typedef struct ubuffer
{
	buffer buffer;
	//this mapping is done to update the buffer from the cpu
	void* mapped;
}ubuffer;

typedef struct render_object
{
	buffer vert_buffer;
	buffer index_buffer;
	//uniform buffers are being updated. We are writing to it in update ubo remember? Thats why we use as many as in Max frames in flight
	//to avoid race
	ubuffer ubo[MAX_FRAMES_IN_FLIGHT];
	//debug purpose
	int id;
	VkDescriptorSet descriptorSet[MAX_FRAMES_IN_FLIGHT];
}render_object;


/*
* Most of this is internal state and unrequired by other structs. Still, I don't know enough about vulkan to want to abstract this away.
* So this will be a megastruct until then.
*/

//https://github.com/KhronosGroup/Vulkan-Samples/blob/main/samples/api/hpp_hello_triangle/hpp_hello_triangle.h
typedef struct yk_frame_data
{
	VkFence in_flight_fence;

	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffers;

	VkSemaphore image_available_semawhore;
	VkSemaphore render_finished_semawhore;

} yk_frame_data;

struct YkRenderer
{
	VmaAllocator allocator;
	VkInstance vk_instance;
	//ToDo(facts 12/24 0341): Does renderer receive a window handle? Or does the renderer own the window? I want to be able to support multiple windows
	//for whatever reason. I will get back to this later. For now, a window handle should be fine.
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

	yk_frame_data frame_data[MAX_FRAMES_IN_FLIGHT];

	clock_t clock;

	uint32_t current_frame;

	render_object render_objects[10];
	i32 num_ro;


#if VK_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};



enum Q_FAM
{
	Q_FAM_GFX,
	Q_FAM_GFX_COMPUTE,
	Q_FAM_PRESENT
};



#endif // !RENDERER_MN_TYPES

#ifndef RENDERER_H
#define RENDERER_H

#include <renderer/mn_types.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <yk_api.h>

#include <vma/vk_mem_alloc.h>
#include <time.h>
#include <yk_window.h>

/*
* Most of this is internal state and unrequired by other structs. Still, I don't know enough about vulkan to want to abstract this away.
* So this will be a megastruct until then.
*/

//https://github.com/KhronosGroup/Vulkan-Samples/blob/main/samples/api/hpp_hello_triangle/hpp_hello_triangle.h
struct yk_frame_data
{
	VkFence in_flight_fence;

	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffers;

	VkSemaphore image_available_semawhore;
	VkSemaphore render_finished_semawhore;
	
};

struct YkRenderer
{
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
	
	VkExtent2D sc_extent;
	VkImage sc_images[max_images];
	VkImageView sc_image_views[max_images];
	
	VkDescriptorPool global_pool;
	
	yk_frame_data frame_data[MAX_FRAMES_IN_FLIGHT];

	clock_t clock;

	uint32_t current_frame;

	render_object render_objects[10];
	i32 num_ro;

	VkDescriptorSet draw_image_desc;
	VkDescriptorSetLayout draw_image_layouts;
	VkPipeline gradient_pp;
	VkPipelineLayout gradient_pp_layouts;

	VkPipelineLayout triangle_pl_layout;
	VkPipeline triangle_pl;

	AllocatedImage draw_image;
	

	VmaAllocator vma_allocator;

	u32 frames_rendered;


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

void yk_free_renderer(YkRenderer* renderer);

void yk_renderer_wait(YkRenderer* renderer);

void yk_renderer_innit(YkRenderer* renderer, struct YkWindow* window);
void yk_renderer_innit_model(YkRenderer* renderer, const vertex vertices[], const u16 indices[], render_object* render_object);

void yk_renderer_raster_draw(YkRenderer* renderer, YkWindow* win);

void yk_renderer_draw(YkRenderer* renderer, YkWindow* win);

void yk_destroy_model(YkRenderer* renderer, render_object* render_object);

VkVertexInputBindingDescription vk_get_binding_desc();
void get_attrib_desc(VkVertexInputAttributeDescription out[]);

b8 yk_recreate_swapchain(YkRenderer* renderer, struct YkWindow* win);

void yk_create_buffer(YkRenderer* ren, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);


typedef struct mvp_matrix mvp_matrix;
#endif
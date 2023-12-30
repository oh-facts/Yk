#ifndef YK_RENDERER_H
#define YK_RENDERER_H


#include <yk.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <yk_math.h>
#include <yk_math.h>
#include <renderer/mn_types.h>


typedef struct YkRenderer YkRenderer;

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

	yk_frame_data frame_data[MAX_FRAMES_IN_FLIGHT];


	uint32_t current_frame;


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


YK_API void yk_free_renderer(YkRenderer* renderer);

YK_API void yk_renderer_wait(YkRenderer* renderer);

YK_API void yk_renderer_innit(YkRenderer* renderer, struct YkWindow* window);
YK_API void yk_renderer_innit_model(YkRenderer* renderer, const vertex vertices[], const u16 indices[], render_object* render_object);
YK_API void yk_renderer_draw_model(YkRenderer* renderer, render_object* render_objects, int num);

YK_API void yk_destroy_model(YkRenderer* renderer, render_object* render_object);

VkVertexInputBindingDescription vk_get_binding_desc();
void get_attrib_desc(VkVertexInputAttributeDescription out[]);


typedef struct mvp_matrix mvp_matrix;
#endif
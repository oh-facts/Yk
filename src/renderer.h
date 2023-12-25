#ifndef YK_RENDERER_H
#define YK_RENDERER_H


#include <yk.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#define VkDEBUG 1

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 0


//Note(facts 0513 12/24): I don't know why I arrived at 3 with this. I'll get back to it later
#define max_images 3

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct YkRenderer YkRenderer;

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

	VkPipelineLayout pipeline_layout;
	VkPipeline gfx_pipeline;

	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffers[MAX_FRAMES_IN_FLIGHT];

	VkSemaphore image_available_semawhores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semawhores[MAX_FRAMES_IN_FLIGHT];
	VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

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

void yk_innit_renderer(YkRenderer* renderer, struct YkWindow* window);

void yk_free_renderer(YkRenderer* renderer);

void vk_draw_frame(YkRenderer* renderer);

void yk_renderer_wait(YkRenderer* renderer);

#endif
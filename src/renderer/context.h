#ifndef YK_RENDERER_CONTEXT_H
#define YK_RENDERER_CONTEXT_H

#include <yk.h>
#include <renderer/common.h>

typedef struct mn_context mn_context;
typedef struct mn_swapchain mn_swapchain;
typedef struct moon moon;
typedef struct mn_device mn_device;

struct mn_context
{
	VkInstance vk_instance;
	VkSurfaceKHR surface;
#if VK_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct mn_swapchain
{
	VkSwapchainKHR swapchain;

	VkImage images[max_images];
	VkImageView image_views[max_images];
};

struct mn_device
{
	VkDevice vk_device;
	VkPhysicalDevice vk_phys_device;
	VkQueue gfx_q;
	i32 qfams[3];
};

struct moon
{
	mn_context context;
	mn_device device;

	VkViewport viewport;
	VkRect2D scissor;
	VkExtent2D extent;
};

void mn_context_innit(mn_context* context, void * surface_context);
void mn_context_free(mn_context* context);

void mn_device_innit(mn_context* context, mn_device* device);

#endif
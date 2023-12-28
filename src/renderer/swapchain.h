#ifndef RENDERER_SWAPCHAIN_H
#define RENDERER_SWAPCHAIN_H

#include "device.h"

typedef struct mn_swapchain mn_swapchain;
typedef struct mn_swapchain_info mn_swapchain_info;

struct mn_swapchain
{
	VkSwapchainKHR swapchain;

	VkImage images[max_images];
	VkImageView image_views[max_images];

	VkViewport viewport;
	VkRect2D scissor;
	VkExtent2D extent;
};

struct mn_swapchain_info
{
	VkSurfaceKHR surface;
	i32 win_size_x;
	i32 win_size_y;
};

void mn_surface_innit(mn_context* context, void* window_handle, VkSurfaceKHR *surface);
void mn_swapchain_innit(mn_context* context, mn_device * device, mn_swapchain_info swapchain_info, mn_swapchain* swapchain);

#endif // !RENDERER_SWAPCHAIN_H
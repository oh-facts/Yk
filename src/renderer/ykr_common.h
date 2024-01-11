#ifndef YKR_COMMON_H
#define YKR_COMMON_H

#define VkDEBUG 1

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 0

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <time.h>
#include <stdio.h>

#include <yk_math.h>

#define max_images 3

#define MAX_FRAMES_IN_FLIGHT 2

struct AllocatedImage 
{
	VkImage image;
	VkImageView imageView;
 	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

#endif // !YKR_COMMON_H

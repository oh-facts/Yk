#ifndef YKR_COMMON_H
#define YKR_COMMON_H

/*
	ToDo(facts): Dont use Vk as prefix. Comes off as vulkan provided macro
*/
#define VkDEBUG 1

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 1
#define FORCE_INTEGRATED 0

#include <pch.h>

#include <yk_memory.h>
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

struct YkBuffer
{
	VkBuffer buffer;
	VmaAllocation alloc;
	VmaAllocationInfo info;
};

struct YkVertex
{
	v3 pos;
	f32 uv_x;
	v3 normal;
	f32 uv_y;
	v4 color;
};

struct YkMeshBuffer
{
	YkBuffer i_buffer;
	YkBuffer v_buffer;
	VkDeviceAddress v_address;
};

struct YkDrawPushConstants
{
	glm::mat4 world_matrix;
	VkDeviceAddress v_buffer;
};


struct geo_surface
{
	u32 start;
	u32 count;
	u32 texture_id;
};

struct texture_asset
{
	AllocatedImage image;
	VkSampler sampler;
	u64 id;
	const char* name;
};

struct material
{
	texture_asset base;
};

struct mesh_asset
{
	const char* name;
	struct YkMeshBuffer buffer;
	struct geo_surface* surfaces;
	u32 num_surfaces;
	glm::mat4 model_mat;
};

struct YkRenderer;

void copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size);
VkImageViewCreateInfo image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);

YkBuffer ykr_create_buffer(VmaAllocator allocator, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
void ykr_destroy_buffer(VmaAllocator allocator, const YkBuffer* buffer);

typedef void (*imm_submit_fn)(VkCommandBuffer cmd);
void ykr_imm_submit(VkDevice device, VkCommandBuffer cmd, VkFence fence, void (*fn)(VkCommandBuffer, void*), void* data, VkQueue queue);

#endif // !YKR_COMMON_H

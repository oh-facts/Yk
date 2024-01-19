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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct YkDrawPushConstants
{
	glm::mat4 world_matrix;
	VkDeviceAddress v_buffer;
};


struct geo_surface
{
	u32 start;
	u32 count;
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

mesh_asset* yk_load_mesh(YkRenderer* renderer, const char* filepath, void* memory, size_t size, size_t* out_num_meshes);
#endif // !YKR_COMMON_H

#ifndef RENDERER_MN_TYPES
#define RENDERER_MN_TYPES

#define VkDEBUG 1

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 0
#include <vma/vk_mem_alloc.h>
#include <time.h>

#include <yk_math.h>
typedef struct vertex vertex;

#define max_images 3

#define MAX_FRAMES_IN_FLIGHT 2

struct vertex
{
	v2 pos;
	v3 color;
};


struct mvp_matrix
{
	m4 model;
	m4 view;
	m4 proj;
};

struct buffer
{
	VkBuffer handle;
	VkDeviceMemory memory;
};

struct ubuffer
{
	buffer buffer;
	//this mapping is done to update the buffer from the cpu
	void* mapped;
};

struct render_object
{
	buffer vert_buffer;
	buffer index_buffer;
	//uniform buffers are being updated. We are writing to it in update ubo remember? Thats why we use as many as in Max frames in flight
	//to avoid race
	ubuffer ubo[MAX_FRAMES_IN_FLIGHT];
	//debug purpose
	int id;
	VkDescriptorSet descriptorSet[MAX_FRAMES_IN_FLIGHT];
};

struct AllocatedImage 
{
	VkImage image;
	VkImageView imageView;
 	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

#endif // !RENDERER_MN_TYPES

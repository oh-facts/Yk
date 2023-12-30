#ifndef RENDERER_MN_TYPES
#define RENDERER_MN_TYPES
#include <yk.h>

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


#endif // !RENDERER_MN_TYPES

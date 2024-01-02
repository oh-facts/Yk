#ifndef YK_RENDERER_H
#define YK_RENDERER_H

#include <renderer/mn_types.h>


typedef struct YkRenderer YkRenderer;


void yk_free_renderer(YkRenderer* renderer);

void yk_renderer_wait(YkRenderer* renderer);

void yk_renderer_innit(YkRenderer* renderer, struct YkWindow* window);
void yk_renderer_innit_model(YkRenderer* renderer, const vertex vertices[], const u16 indices[], render_object* render_object);

void yk_renderer_update(YkRenderer* renderer, YkWindow* win);

void yk_destroy_model(YkRenderer* renderer, render_object* render_object);

VkVertexInputBindingDescription vk_get_binding_desc();
void get_attrib_desc(VkVertexInputAttributeDescription out[]);

b8 yk_recreate_swapchain(YkRenderer* renderer, struct YkWindow* win);

void yk_create_buffer(YkRenderer* ren, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);


typedef struct mvp_matrix mvp_matrix;
#endif
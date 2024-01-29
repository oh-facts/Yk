#ifndef YK_DESCRIPTORS_H
#define YK_DESCRIPTORS_H

#include <renderer/ykr_common.h>

struct YkDescriptorSet
{
	int temp;
};

void desc_pool_innit(VkDevice device, VkDescriptorPool* pool);
void desc_set_innit(VkDevice device, VkDescriptorSet* set, VkDescriptorPool pool, VkDescriptorSetLayout* layouts, YkBuffer* buffer, size_t ubo_size);
void desc_layout_innit(VkDevice device, VkDescriptorSetLayout* layout);

void ubo_update(VmaAllocator allocator, YkBuffer* buffer, void* ubo, size_t size);

#endif
#ifndef YK_DESCRIPTORS_H
#define YK_DESCRIPTORS_H

#include <renderer/ykr_common.h>
#include <renderer/ykr_debug_util.h>
struct YkDescriptorSet
{
	int temp;
};

void ykr_desc_layout_innit(VkDevice device, VkDescriptorSetLayoutBinding* bindings, u32 num_bindings, VkDescriptorSetLayout* layout);
void ykr_desc_pool_innit(VkDevice device, u32 max_sets, VkDescriptorPoolSize* pool_sizes, u32 num_pools_sizes, VkDescriptorPool* pool);

void desc_set_innit(VkDevice device, VkDescriptorSet* set, VkDescriptorPool pool, VkDescriptorSetLayout* layouts);


void ubo_update(VmaAllocator allocator, YkBuffer* buffer, void* ubo, size_t size);

#endif
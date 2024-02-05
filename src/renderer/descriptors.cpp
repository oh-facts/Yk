#include <renderer/descriptors.h>

void ykr_desc_layout_innit(VkDevice device, VkDescriptorSetLayoutBinding* bindings, u32 num_bindings, VkDescriptorSetLayout* layout)
{
    VkDescriptorSetLayoutCreateInfo desc_layout_info = {};
    desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    desc_layout_info.bindingCount = num_bindings;
    desc_layout_info.pBindings = bindings;

    VkResultAssert(vkCreateDescriptorSetLayout(device, &desc_layout_info, 0, layout),"desc set layout");
}

void ykr_desc_pool_innit(VkDevice device, u32 max_sets, VkDescriptorPoolSize* pool_sizes, u32 num_pools_sizes, VkDescriptorPool* pool)
{
    VkDescriptorPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext = 0;
    info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    info.maxSets = max_sets;
    info.poolSizeCount = num_pools_sizes;
    info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(device, &info, 0, pool);
}

void desc_set_innit(VkDevice device, VkDescriptorSet* set, VkDescriptorPool pool, VkDescriptorSetLayout* layouts)
{
    VkDescriptorSetAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext = 0;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = layouts;

    VkResultAssert(vkAllocateDescriptorSets(device, &info, set), "desc set alloc");
    
}



void ubo_update(VmaAllocator allocator, YkBuffer* buffer, void* ubo, size_t size)
{
    void* data = 0;
    vmaMapMemory(allocator, buffer->alloc, &data);

    memcpy(data, ubo, size);

    vmaUnmapMemory(allocator, buffer->alloc);
}

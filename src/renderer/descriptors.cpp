#include <renderer/descriptors.h>



void desc_pool_innit(VkDevice device, VkDescriptorPool* pool)
{
    constexpr size_t pool_size = 1;
    VkDescriptorPoolSize pool_sizes[pool_size] = {};

    for (u32 i = 0; i < pool_size; i++)
    {
        pool_sizes[i].descriptorCount = 20 * MAX_FRAMES_IN_FLIGHT;
        pool_sizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }

    VkDescriptorPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext = 0;
    info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    info.maxSets = 20 * MAX_FRAMES_IN_FLIGHT;
    info.poolSizeCount = pool_size;
    info.pPoolSizes = pool_sizes;


    vkCreateDescriptorPool(device, &info, 0, pool);
}

void desc_set_innit(VkDevice device, VkDescriptorSet* set, VkDescriptorPool pool, VkDescriptorSetLayout* layouts, YkBuffer* buffer, size_t ubo_size)
{
    VkDescriptorSetAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext = 0;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = layouts;

    VkResultAssert(vkAllocateDescriptorSets(device, &info, set), "desc set alloc");

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer->buffer;
    buffer_info.offset = 0;
    buffer_info.range = ubo_size;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = *set;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(device, 1, &write, 0, 0);
}

void desc_layout_innit(VkDevice device, VkDescriptorSetLayout* layout, VkShaderStageFlags flags)
{   
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorCount = 1;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.stageFlags = flags;

    VkDescriptorSetLayoutCreateInfo desc_layout_info = {};
    desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    desc_layout_info.bindingCount = 1;
    desc_layout_info.pBindings = &binding;

    /*
    const VkDescriptorBindingFlagsEXT flags =
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
    binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    binding_flags.bindingCount = 1;
    binding_flags.pBindingFlags = &flags;
    desc_layout_info.pNext = &binding_flags;
    */
    vkCreateDescriptorSetLayout(device, &desc_layout_info, 0, layout);
}


void ubo_update(VmaAllocator allocator, YkBuffer* buffer, void* ubo, size_t size)
{
    void* data = 0;
    vmaMapMemory(allocator, buffer->alloc, &data);

    memcpy(data, ubo, size);

    vmaUnmapMemory(allocator, buffer->alloc);
}

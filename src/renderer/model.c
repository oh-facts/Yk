#include "model.h"
#include <stdint.h>
#include <stddef.h>

clock_t start_time;

void copyBuffer(VkDevice device, VkQueue gfx_q, VkCommandPool cmd_pool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmd_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = { 0 };
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(gfx_q, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(gfx_q);

    vkFreeCommandBuffers(device, cmd_pool, 1, &commandBuffer);

}

u32 findMemoryType(VkPhysicalDevice physical_device, u32 typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Assert(false, "Failed to find suitable memory type!");
    return 69420;
}

void create_buffer(VkDevice device, VkPhysicalDevice phys_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
    VkBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResultAssert(vkCreateBuffer(device, &bufferInfo, 0, buffer), "Created buffer")

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(phys_device, memRequirements.memoryTypeBits, properties);

    VkResultAssert(vkAllocateMemory(device, &allocInfo, 0, bufferMemory), "Buffer memory allocation");

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);

}


void updateUniformBuffer(yk_renderer* renderer, uint32_t currentImage)
{
    clock_t current_time = clock();
    f32 time = (f32)(current_time - start_time) / CLOCKS_PER_SEC;

    ubo ubo = { 0 };

    ubo.model = yk_m4_rotate(yk_m4_identity(), time * DEG_TO_RAD * 90.f, (v3) { 0, 0, 1 });
    ubo.view = yk_m4_look_at((v3) { 2, 2, 2 }, (v3) { 0, 0, 0 }, (v3) { 0, 0, 1 });
    ubo.proj = yk_m4_perspective(DEG_TO_RAD * 45., renderer->swapchain.extent.width / (f32)renderer->swapchain.extent.height, 0.1f, 10.0f);


    ubo.proj.e[1][1] *= -1;

    memcpy(renderer->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}



void yk_create_vert_buffer(yk_renderer* ren, vertex* vertices, buffer *buffer)
{//we

    VkDeviceSize bufferSize = sizeof(vertices[0]) * 4;

    VkBuffer staging_buffer = { 0 };
    VkDeviceMemory staging_buffer_memory = { 0 };

    create_buffer(ren->device.handle, ren->device.phys_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void* data;
    vkMapMemory(ren->device.handle, staging_buffer_memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);

    create_buffer(ren->device.handle, ren->device.phys_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer->handle, &buffer->memory);

    copyBuffer(ren->device.handle, ren->device.gfx_q, ren->cmd_pool, staging_buffer, buffer->handle, buffer->memory);

    vkDestroyBuffer(ren->device.handle, staging_buffer, 0);
    vkFreeMemory(ren->device.handle, staging_buffer_memory, 0);
}

void yk_create_index_buffer(yk_renderer* ren, u32 indices[], buffer* buffer)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * 6;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(ren->device.handle, ren->device.phys_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, & stagingBuffer, & stagingBufferMemory);

    void* data;
    vkMapMemory(ren->device.handle, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(ren->device.handle, stagingBufferMemory);

    create_buffer(ren->device.handle, ren->device.phys_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer->handle, &buffer->memory);

    copyBuffer(ren->device.handle, ren->device.gfx_q, ren->cmd_pool, stagingBuffer, buffer->handle, buffer->memory);

    vkDestroyBuffer(ren->device.handle, stagingBuffer, 0);
    vkFreeMemory(ren->device.handle, stagingBufferMemory, 0);
}

void yk_model_init(yk_renderer* ren, vertex vertices[], u16 indices[], model* modela)
{
    start_time = clock();
    
    yk_create_vert_buffer(ren,vertices, &modela->vert_buffer);


    yk_create_index_buffer(ren,indices,&modela->index_buffer);
}


void createUniformBuffers(yk_renderer* renderer)
{
    VkDeviceSize bufferSize = sizeof(ubo);


    for (size_t i = 0; i < 2; i++) {
        create_buffer(renderer->device.handle, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &renderer->uniformBuffers[i], &renderer->uniformBuffersMemory[i]);

        vkMapMemory(renderer->device.handle, renderer->uniformBuffersMemory[i], 0, bufferSize, 0, &renderer->uniformBuffersMapped[i]);
    }
}
#include <renderer/renderer.h>
#include <renderer/model.h>

void mn_create_desc_pool(yk_renderer* renderer);
void mn_create_desc_layout(mn_device* device, VkDescriptorSetLayout* layout);
void mn_create_desc_sets(yk_renderer* renderer);


void yk_create_cmd_pool(yk_renderer* renderer);

void get_attrib_desc(VkVertexInputAttributeDescription out[]);
VkVertexInputBindingDescription vk_get_binding_desc();

b8 yk_recreate_swapchain(yk_renderer* renderer);
void yk_renderer_wait(yk_renderer* renderer);

#include <yk_math.h>




void yk_create_cmd_buffer(yk_renderer* renderer)
{
    VkCommandBufferAllocateInfo vk_cmd_buffer_alloc_info = { 0 };
    vk_cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vk_cmd_buffer_alloc_info.pNext = 0;
    vk_cmd_buffer_alloc_info.commandPool = renderer->cmd_pool;
    vk_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk_cmd_buffer_alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;


    VkResultAssert(vkAllocateCommandBuffers(renderer->device.handle, &vk_cmd_buffer_alloc_info, renderer->cmd_buffers), "Command Buffer allocation");

}

void yk_create_sync_objs(yk_renderer* renderer)
{
    //https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation

    //Some 7 stuff. I need semawhores

    //if semaphores aren't extended with semaphore types, they will be binary


    VkSemaphoreCreateInfo vk_semawhore_create_info = { 0 };
    vk_semawhore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk_semawhore_create_info.pNext = 0;
    vk_semawhore_create_info.flags = 0;

    VkFenceCreateInfo vk_fence_create_info = { 0 };
    vk_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResultAssert(vkCreateSemaphore(renderer->device.handle, &vk_semawhore_create_info, 0, &renderer->image_available_semawhores[i]), "Image ready semaphore");
        VkResultAssert(vkCreateSemaphore(renderer->device.handle, &vk_semawhore_create_info, 0, &renderer->render_finished_semawhores[i]), "Render finished semaphore");
        VkResultAssert(vkCreateFence(renderer->device.handle, &vk_fence_create_info, 0, &renderer->in_flight_fences[i]), "flight fence");
    }

}


void yk_renderer_wait(yk_renderer* renderer)
{
    vkDeviceWaitIdle(renderer->device.handle);
}

b8 yk_recreate_swapchain(yk_renderer* renderer)
{
    if (!renderer->win_data->is_running)
    {
        return false;
    }

    vkDeviceWaitIdle(renderer->device.handle);

    while (renderer->win_data->is_minimized)
    {
        yk_window_poll();
    }

    for (i32 i = 0; i < max_images; i++)
    {
        vkDestroyImageView(renderer->device.handle, renderer->swapchain.image_views[i], 0);
    }

    vkDestroySwapchainKHR(renderer->device.handle, renderer->swapchain.swapchain, 0);

    mn_swapchain_innit(&renderer->context, &renderer->device, (mn_swapchain_info) {.surface = renderer->surface, .win_size_x = renderer->win_data->x, .win_size_y = renderer->win_data->y},
    &renderer->swapchain);


    return true;
}



void yk_renderer_innit(yk_renderer* self, window_data * win_data)
{

    const vertex vertices[] = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {163 / 255.f, 163 / 255.f, 163 / 255.f}},
        {{0.5f, 0.5f}, {1.f, 1.f, 1.f}},
        {{-0.5f, 0.5f}, {128 / 255.f, 0.f, 128 / 255.f}}
    };

    const u16 indices[] = {
        0, 1, 2, 2, 3, 0
    };


    self->win_data = win_data;
    mn_context_innit(&self->context);

    mn_surface_innit(&self->context, win_data->win_handle, &self->surface);

    mn_device_innit(&self->context, self->surface, &self->device);

    mn_swapchain_innit(&self->context, &self->device, (mn_swapchain_info) { .win_size_x = 800, .win_size_y = 600, .surface = self->surface }, &self->swapchain);

    mn_create_desc_pool(self);

    mn_create_desc_layout(&self->device, &self->descriptorSetLayout);

    VkVertexInputBindingDescription binding_desc = vk_get_binding_desc();
    VkVertexInputAttributeDescription attrib_desc[2];
    get_attrib_desc(attrib_desc);

    mn_raster_pipeline_innit(&self->device, &self->swapchain, &self->pipeline, &self->descriptorSetLayout, &binding_desc, attrib_desc );

    yk_create_cmd_pool(self);

    yk_model_init(self, vertices, indices, &self->model);

    createUniformBuffers(self);

    mn_create_desc_pool(self);
    mn_create_desc_sets(self);

    yk_create_cmd_buffer(self);
    yk_create_sync_objs(self);
}

void yk_renderer_draw(yk_renderer* self)
{
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(self->device.handle, "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(self->device.handle, "vkCmdEndRenderingKHR");


    VkResultAssert(vkWaitForFences(self->device.handle, 1, &self->in_flight_fences[self->current_frame], VK_TRUE, UINT64_MAX), "Wait for fences")

        uint32_t imageIndex = -1;

    if (vkAcquireNextImageKHR(self->device.handle, self->swapchain.swapchain, UINT64_MAX,
        self->image_available_semawhores[self->current_frame],
        VK_NULL_HANDLE, &imageIndex) == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if (yk_recreate_swapchain(self) == false)
        {
            return;
        }
    }

    updateUniformBuffer(self, self->current_frame);
    VkResultAssert(vkResetFences(self->device.handle, 1, &self->in_flight_fences[self->current_frame]), "Reset fences");



    VkResultAssert(vkResetCommandBuffer(self->cmd_buffers[self->current_frame], 0), "Cmd buffer reset");

    //command buffer record


    VkRenderingAttachmentInfoKHR vk_color_attachment = { 0 };
    vk_color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_color_attachment.imageView = self->swapchain.image_views[imageIndex];
    vk_color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vk_color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vk_color_attachment.clearValue.color = (VkClearColorValue){ 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingAttachmentInfoKHR vk_depth_attachment = { 0 };
    vk_depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_depth_attachment.imageView = self->swapchain.image_views[1];
    vk_depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    vk_depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vk_depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vk_depth_attachment.clearValue.depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };

    VkRenderingAttachmentInfoKHR vk_stencil_attachment = { 0 };
    vk_stencil_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_stencil_attachment.imageView = self->swapchain.image_views[2];
    vk_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    vk_stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vk_stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vk_stencil_attachment.clearValue.depthStencil = (VkClearDepthStencilValue){ 0.0f, 0 };


    VkRenderingInfoKHR vk_rendering_info = { 0 };
    vk_rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    vk_rendering_info.pNext = 0;
    vk_rendering_info.flags = 0;
    vk_rendering_info.renderArea = self->swapchain.scissor;
    vk_rendering_info.layerCount = 1;
    vk_rendering_info.viewMask = 0;
    vk_rendering_info.colorAttachmentCount = 1;
    vk_rendering_info.pColorAttachments = &vk_color_attachment;
    vk_rendering_info.pDepthAttachment = VK_NULL_HANDLE; //&vk_depth_attachment;
    vk_rendering_info.pStencilAttachment = VK_NULL_HANDLE; //&vk_stencil_attachment;

    VkCommandBufferBeginInfo vk_cmd_buffer_begin_info = { 0 };
    vk_cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_cmd_buffer_begin_info.pNext = 0;
    //0 is fine. flags are mean for specific cases
    vk_cmd_buffer_begin_info.flags = 0;
    //for secondary buffers
    vk_cmd_buffer_begin_info.pInheritanceInfo = 0;

    VkResultAssert(vkBeginCommandBuffer(self->cmd_buffers[self->current_frame], &vk_cmd_buffer_begin_info), "Command buffer begin");

    // Begin rendering
    vkCmdBeginRenderingKHR(self->cmd_buffers[self->current_frame], &vk_rendering_info);

    vkCmdBindPipeline(self->cmd_buffers[self->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, self->pipeline.handle);


    vkCmdSetViewport(self->cmd_buffers[self->current_frame], 0, 1, &self->swapchain.viewport);
    vkCmdSetScissor(self->cmd_buffers[self->current_frame], 0, 1, &self->swapchain.scissor);


    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(self->cmd_buffers[self->current_frame], 0, 1, &self->model.vert_buffer, offsets);
    vkCmdBindIndexBuffer(self->cmd_buffers[self->current_frame], &self->model.index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(self->cmd_buffers[self->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS,
    self->pipeline.pipeline_layout, 0, 1, &self->descriptorSets[self->current_frame], 0, 0);
    vkCmdDrawIndexed(self->cmd_buffers[self->current_frame], 6, 1, 0, 0, 0);


    // End rendering
    vkCmdEndRenderingKHR(self->cmd_buffers[self->current_frame]);

    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    Assert(imageIndex < max_images, "Too many images.")

    barrier.image = self->swapchain.images[imageIndex];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    vkCmdPipelineBarrier(
        self->cmd_buffers[self->current_frame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, 0,
        0, 0,
        1, &barrier
    );

    VkResultAssert(vkEndCommandBuffer(self->cmd_buffers[self->current_frame]), "Command buffer end");

    //command buffer recording over

    VkSubmitInfo vk_submit_info = { 0 };
    vk_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vk_submit_info.pNext = 0;

    VkSemaphore vk_wait_semawhores[] = { self->image_available_semawhores[self->current_frame] };
    VkPipelineStageFlags vk_wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    vk_submit_info.waitSemaphoreCount = 1;
    vk_submit_info.pWaitSemaphores = vk_wait_semawhores;
    vk_submit_info.pWaitDstStageMask = vk_wait_stages;

    vk_submit_info.commandBufferCount = 1;
    vk_submit_info.pCommandBuffers = &self->cmd_buffers[self->current_frame];

    VkSemaphore vk_signal_semawhores[] = { self->render_finished_semawhores[self->current_frame] };
    vk_submit_info.signalSemaphoreCount = 1;
    vk_submit_info.pSignalSemaphores = vk_signal_semawhores;

    VkResultAssert(vkQueueSubmit(self->device.gfx_q, 1, &vk_submit_info, self->in_flight_fences[self->current_frame]), "Draw command buffer submitted");

    VkPresentInfoKHR vk_present_info = { 0 };
    vk_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    vk_present_info.waitSemaphoreCount = 1;
    vk_present_info.pWaitSemaphores = vk_signal_semawhores;

    VkSwapchainKHR vk_swapchains[] = { self->swapchain.swapchain };
    vk_present_info.swapchainCount = 1;
    vk_present_info.pSwapchains = vk_swapchains;
    vk_present_info.pImageIndices = &imageIndex;

    vk_present_info.pResults = 0;

    //present q same as graphics for now
    VkResult qpresent_result = vkQueuePresentKHR(self->device.gfx_q, &vk_present_info);

    if (qpresent_result == VK_ERROR_OUT_OF_DATE_KHR || qpresent_result == VK_SUBOPTIMAL_KHR)
    {
        if (yk_recreate_swapchain(self) == false)
        {
            return;
        }
    }

    self->current_frame = (self->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}



void mn_create_desc_pool(yk_renderer* renderer)
{
    VkDescriptorPoolSize poolSize = { 0 };

    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = 2;

    VkResultAssert(vkCreateDescriptorPool(renderer->device.handle, &poolInfo, 0, &renderer->descriptorPool), "descripter pool")

}

void mn_create_desc_layout(mn_device* device, VkDescriptorSetLayout* layout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = { 0 };
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = { 0 };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkResultAssert(vkCreateDescriptorSetLayout(device->handle, &layoutInfo, 0, layout), "descr set layout")
}

void mn_create_desc_sets(yk_renderer* renderer)
{
    VkDescriptorSetLayout layouts[2] = { renderer->descriptorSetLayout,renderer->descriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer->descriptorPool;
    allocInfo.descriptorSetCount = 2;
    allocInfo.pSetLayouts = layouts;

    VkResultAssert(vkAllocateDescriptorSets(renderer->device.handle, &allocInfo, &renderer->descriptorSets[0]), "descriptor sets")

        for (size_t i = 0; i < 2; i++) {
            VkDescriptorBufferInfo bufferInfo = { 0 };
            bufferInfo.buffer = renderer->uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(ubo);

            VkWriteDescriptorSet descriptorWrite = { 0 };
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = renderer->descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(renderer->device.handle, 1, &descriptorWrite, 0, 0);
        }
}

VkVertexInputBindingDescription vk_get_binding_desc()
{
    VkVertexInputBindingDescription bindingDescription = { 0 };

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

void get_attrib_desc(VkVertexInputAttributeDescription out[])
{
    out[0].binding = 0;
    out[0].location = 0;
    out[0].format = VK_FORMAT_R32G32_SFLOAT;
    out[0].offset = offsetof(vertex, pos);

    out[1].binding = 0;
    out[1].location = 1;
    out[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    out[1].offset = offsetof(vertex, color);

}


void yk_create_cmd_pool(yk_renderer* renderer)
{
    VkCommandPoolCreateInfo vk_cmd_pool_create_info = { 0 };
    vk_cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vk_cmd_pool_create_info.pNext = 0;
    vk_cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_cmd_pool_create_info.queueFamilyIndex = Q_FAM_GFX;

    VkCommandPool vk_cmd_pool;
    VkResultAssert(vkCreateCommandPool(renderer->device.handle, &vk_cmd_pool_create_info, 0, &vk_cmd_pool), "Command pool creation");

    renderer->cmd_pool = vk_cmd_pool;


}
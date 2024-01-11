#include <renderer/renderer.h>
#include <renderer/ykr_util.h>
#include <renderer/ykr_instance.h>

/*
    -------------------------------
*/

void yk_cmd_innit(YkRenderer* renderer);

void yk_create_sync_objs(YkRenderer* renderer);
b8 yk_recreate_swapchain(YkRenderer* renderer, YkWindow* win);

void yk_cleanup_swapchain(YkRenderer* renderer);

void pipeline_innit(YkRenderer* renderer);
void gradient_pipeline(YkRenderer* renderer);
void triangle_pipeline(YkRenderer* renderer);

/*
 -------util-------
*/

VkCommandBufferBeginInfo yk_cmd_buffer_begin_info_create(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = 0;
    info.pInheritanceInfo = 0;
    info.flags = flags;
    return info;
}

VkSemaphoreSubmitInfo yk_semawhore_submit_info_create(VkPipelineStageFlags2 stage_mask, VkSemaphore semawhore)
{
    VkSemaphoreSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    submitInfo.pNext = 0;
    submitInfo.semaphore = semawhore;
    submitInfo.stageMask = stage_mask;
    submitInfo.deviceIndex = 0;
    submitInfo.value = 1;

    return submitInfo;
}

VkCommandBufferSubmitInfo yk_cmd_buffer_submit_info_create(VkCommandBuffer cmd)
{
    VkCommandBufferSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info.pNext = 0;
    info.commandBuffer = cmd;
    info.deviceMask = 0;

    return info;

}

VkSubmitInfo2 yk_submit_info_create(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
{

    VkSubmitInfo2 out = { };
    out.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    out.pNext = 0;
    out.flags = 0;

    out.waitSemaphoreInfoCount = waitSemaphoreInfo ? 1 : 0;
    out.pWaitSemaphoreInfos = waitSemaphoreInfo;

    out.signalSemaphoreInfoCount = signalSemaphoreInfo ? 1 : 0;
    out.pSignalSemaphoreInfos = signalSemaphoreInfo;

    out.commandBufferInfoCount = 1;
    out.pCommandBufferInfos = cmd;

    return out;

}


VkDescriptorSetLayoutBinding create_desc_binding(u32 binding, VkDescriptorType type, VkShaderStageFlags flags)
{
    VkDescriptorSetLayoutBinding out = { };
    out.binding = binding;
    out.descriptorType = type;
    out.descriptorCount = 1;
    out.stageFlags = flags;
  
    return out;
}

VkDescriptorSetLayout create_desc_set_layout(VkDevice device, VkDescriptorSetLayoutBinding bindings[], u32 num)
{
    VkDescriptorBindingFlags flage = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlags = {};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    bindingFlags.bindingCount = 1;
    bindingFlags.pBindingFlags = &flage;
    
    VkDescriptorSetLayoutCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = num;
    info.pBindings = bindings;
    info.pNext = &bindingFlags;

    VkDescriptorSetLayout out = { };

    VkResultAssert(vkCreateDescriptorSetLayout(device,&info, 0 , &out), "Desc set layout");
    
    return out;
}

void desc_pool_innit(VkDevice device, VkDescriptorType types[], u32 desc_count, VkDescriptorPool *pool)
{
    constexpr u32 pool_size = 1;
    VkDescriptorPoolSize pool_sizes[pool_size] = {};
    for(i32 i = 0; i < desc_count; i ++)
    {
        pool_sizes[i].descriptorCount = 1;
        pool_sizes[i].type = types[i];
    }
    
    VkDescriptorPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.maxSets = 1;
    info.poolSizeCount = desc_count;
    info.pPoolSizes = pool_sizes;

    VkResultAssert(vkCreateDescriptorPool(device, &info, 0, pool), "w_");
}

VkDescriptorSet desc_set_allocate(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorSetCount = 1;
    info.descriptorPool = pool;
    info.pSetLayouts = &layout;
  

    VkDescriptorSet out = {};
    VkResultAssert(vkAllocateDescriptorSets(device, &info, &out), " ");

    return out;
}


void shader_module_innit(VkDevice device, const char* filename, VkShaderModule* shader_module)
{

    size_t len = 0;

    //ToDo(facts): Use arena
    //This allocation is literally what arenas are for
    const char* shader_code = yk_read_binary_file(filename, &len);

    VkShaderModuleCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = len;
    info.pCode = (u32*)shader_code;

    VkResultAssert(vkCreateShaderModule(device, &info, 0, shader_module), "Shader Module Creation");

    free((char*)shader_code);
}


/*
    I take an array but atm I only support one
*/
VkPipeline yk_create_raster_pipeline(VkDevice device, const char* vert_path, const char* frag_path, VkPipelineLayout* layout)
{
    VkGraphicsPipelineCreateInfo gfx_pl_info = {};
    gfx_pl_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    

    //shader stage info 
    //------------------
    VkShaderModule vert_shader = {};
    shader_module_innit(device, vert_path, &vert_shader);

    VkShaderModule frag_shader = {};
    shader_module_innit(device, frag_path, &frag_shader);

    //ToDo(facts): Make a shader stage creator thing.
    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader;
    frag_shader_stage_info.pName = "main";

    constexpr i32 shader_stages_count = 2;
    VkPipelineShaderStageCreateInfo shader_stages[shader_stages_count] = { vert_shader_stage_info, frag_shader_stage_info };

    //------------------

    //vertex input
    //------------------
    //we are doing vertex pulling now. So we wont send vertex data directly
    VkPipelineVertexInputStateCreateInfo vert_input = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    //------------------


    
    //Note(facts 12/23 0223) : Complete pipeline. Then rendering.


    
    //Input assembly
    //------------------
    VkPipelineInputAssemblyStateCreateInfo input_asm = { };
    input_asm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_asm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_asm.primitiveRestartEnable = VK_FALSE;
    //------------------

    //Not using tesselation
    //-----------------

    //view port state
    //------------------
    //Since this is dynamic. We are defaulting values (it will be ignored anyways). Count values matter but we're using 1 for now
    VkPipelineViewportStateCreateInfo viewport_state = { };
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;
    //------------------


    //Rasterization
    //------------------
    VkPipelineRasterizationStateCreateInfo rasterizer = { };
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    //Note(facts 12/23 2:20): come back to this later
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    //------------------

    //Multisampling
    //------------------
    VkPipelineMultisampleStateCreateInfo multisampling = { };
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = 0;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    //------------------

    //Depth stencil
    //------------------


    //------------------

    //color blend stuff
    //------------------
    //These values are default for no blending
    VkPipelineColorBlendAttachmentState color_blend_attatchment = { };
    color_blend_attatchment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attatchment.blendEnable = VK_FALSE;
    color_blend_attatchment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attatchment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attatchment.alphaBlendOp = VK_BLEND_OP_ADD;

    //thes values are default for yes blending
    /*
    {
        color_blend_attatchment.blendEnable = VK_TRUE;
        color_blend_attatchment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attatchment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attatchment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    */

    //ToDo(facts): I have no fucking idea what this means
    //I just want a triangle

    VkPipelineColorBlendStateCreateInfo color_blending = { };
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attatchment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    //------------------

 
    //Dynamic state
    //------------------
    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn_state_create_info = { };
    dyn_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_state_create_info.dynamicStateCount = 2;
    dyn_state_create_info.pDynamicStates = dynamic_states;
    //------------------

    //layout
    //------------------
    //THis is where desc set data goes
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    VkResultAssert(vkCreatePipelineLayout(device, &layout_info, 0, layout), "pipeline layout creation")
    //------------------

    gfx_pl_info.stageCount = shader_stages_count;
    gfx_pl_info.pStages = shader_stages;
    gfx_pl_info.pVertexInputState = &vert_input;
    gfx_pl_info.pInputAssemblyState = &input_asm;
    gfx_pl_info.pTessellationState = 0;
    gfx_pl_info.pViewportState = &viewport_state;
    gfx_pl_info.pRasterizationState = &rasterizer;
    gfx_pl_info.pMultisampleState = &multisampling;
    gfx_pl_info.pDepthStencilState = 0;
    gfx_pl_info.pColorBlendState = &color_blending;
    gfx_pl_info.pDynamicState = &dyn_state_create_info;
    gfx_pl_info.layout = *layout;

    VkPipeline out = {};
    vkCreateGraphicsPipelines(device, 0, 1, &gfx_pl_info, 0, &out);

    vkDestroyShaderModule(device, frag_shader, 0);
    vkDestroyShaderModule(device, vert_shader, 0);

    return out;
}

// -----------------

void yk_desc_innit(YkRenderer* renderer)
{
    //compute desc set.
    //we only need a storage image layout
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    
    VkDescriptorSetLayoutBinding binding = create_desc_binding(0, type, VK_SHADER_STAGE_COMPUTE_BIT);
    
    renderer->draw_image_layouts = create_desc_set_layout(renderer->device,&binding, 1);



    desc_pool_innit(renderer->device, &type, 1 ,&renderer->global_pool);

    renderer->draw_image_desc = desc_set_allocate(renderer->device,renderer->global_pool,renderer->draw_image_layouts);

    VkDescriptorImageInfo img_info = {};
    img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_info.imageView = renderer->draw_image.imageView;
    
    VkWriteDescriptorSet draw_img_write = {};
    draw_img_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    draw_img_write.dstBinding = 0;
    draw_img_write.descriptorCount = 1;
    draw_img_write.dstSet = renderer->draw_image_desc;
    draw_img_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    draw_img_write.pImageInfo = &img_info;

    vkUpdateDescriptorSets(renderer->device, 1, &draw_img_write, 0 , 0);
}

void gradient_pipeline(YkRenderer* renderer)
{
    VkPipelineLayoutCreateInfo compute_layout = {};
    compute_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    compute_layout.pSetLayouts = &renderer->draw_image_layouts;
    compute_layout.setLayoutCount = 1;


    VkPushConstantRange push_constant = {};
    push_constant.offset = 0;
    push_constant.size = sizeof(ComputePushConstants);
    push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    compute_layout.pushConstantRangeCount = 1;
    compute_layout.pPushConstantRanges = &push_constant;

    VkResultAssert(vkCreatePipelineLayout(renderer->device, &compute_layout, 0, &renderer->gradient_pp_layouts), "w");

    VkShaderModule compute_module = {};
    shader_module_innit(renderer->device, "res/gradient.comp.spv",&compute_module);

    VkPipelineShaderStageCreateInfo comp_shader_stage_info = {};
    comp_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    comp_shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    comp_shader_stage_info.module = compute_module;
    comp_shader_stage_info.pName = "main";

    VkComputePipelineCreateInfo comp_pp_info = {};
    comp_pp_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    comp_pp_info.layout = renderer->gradient_pp_layouts;
    comp_pp_info.stage = comp_shader_stage_info;
    
    VkResultAssert(vkCreateComputePipelines(renderer->device, 0, 1, &comp_pp_info, 0, &renderer->gradient_pp), "w");

    vkDestroyShaderModule(renderer->device, compute_module,0);
}

void triangle_pipeline(YkRenderer* renderer)
{
   renderer->triangle_pl = yk_create_raster_pipeline(renderer->device, "res/default.vert.spv", "res/default.frag.spv", &renderer->triangle_pl_layout);

}


void pipeline_innit(YkRenderer* renderer)
{
    gradient_pipeline(renderer);
    triangle_pipeline(renderer);
}


/*
    Render commands.
    When using the renderer backend to render. It is common to just send commands to the renderer.
*/

void yk_renderer_draw_bg(YkRenderer* renderer, VkCommandBuffer cmd)
{
    /*
    VkClearColorValue clearValue;
    float flash = abs(sin(renderer->frames_rendered / 120.f));
    clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

    VkImageSubresourceRange clear_range = {
                                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                   .baseMipLevel = 0,
                                                   .levelCount = VK_REMAINING_MIP_LEVELS,
                                                   .baseArrayLayer = 0,
                                                   .layerCount = VK_REMAINING_ARRAY_LAYERS
    };

    vkCmdClearColorImage(cmd, renderer->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clear_range);
    */

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->gradient_pp);
   vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,renderer->gradient_pp_layouts, 0, 1, &renderer->draw_image_desc,0,0);

   ComputePushConstants push = {};
   push.data1 = v4{ 1.0,0.0, 1.0,1.0 };
   push.data2 = v4{ 0.0,0.0, 1.0,1.0 };

   vkCmdPushConstants(cmd, renderer->gradient_pp_layouts, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
   
   vkCmdDispatch(cmd, renderer->draw_image.imageExtent.width / 16.0 , renderer->draw_image.imageExtent.height / 16.0, 1);
}

void yk_renderer_draw_triangle(YkRenderer* renderer, VkCommandBuffer cmd)
{
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(renderer->device, "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(renderer->device, "vkCmdEndRenderingKHR");

    VkRenderingAttachmentInfoKHR vk_color_attachment = { };
    vk_color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_color_attachment.imageView = renderer->draw_image.imageView;
    vk_color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    vk_color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vk_color_attachment.clearValue.color = VkClearColorValue{ 1.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingInfoKHR vk_rendering_info = { };
    vk_rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    vk_rendering_info.pNext = 0;
    vk_rendering_info.flags = 0;
    vk_rendering_info.renderArea = renderer->scissor;
    vk_rendering_info.layerCount = 1;
    vk_rendering_info.viewMask = 0;
    vk_rendering_info.colorAttachmentCount = 1;
    vk_rendering_info.pColorAttachments = &vk_color_attachment;
    vk_rendering_info.pDepthAttachment = VK_NULL_HANDLE; //&vk_depth_attachment;
    vk_rendering_info.pStencilAttachment = VK_NULL_HANDLE; //&vk_stencil_attachment;

    vkCmdBeginRenderingKHR(cmd, &vk_rendering_info);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->triangle_pl);

    vkCmdSetViewport(cmd, 0, 1, &renderer->viewport);
    vkCmdSetScissor(cmd, 0, 1, &renderer->scissor);

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRendering(cmd);
}


#define VMA_DEBUG_LOG
#include <vma/vk_mem_alloc.h>

void yk_renderer_innit(YkRenderer* renderer, struct YkWindow* window)
{


    renderer->current_frame = 0;
    //---pure boiler plate ---//
    yk_innit_vulkan(renderer);
    yk_create_surface(renderer, window->win_handle);
    yk_pick_physdevice(renderer);
    yk_create_device(renderer);
   
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = renderer->phys_device;
    allocatorInfo.device = renderer->device;
    allocatorInfo.instance = renderer->vk_instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VkResultAssert(vmaCreateAllocator(&allocatorInfo, &renderer->vma_allocator), "Created Allocator");

    yk_find_queues(renderer);
    yk_create_swapchain(renderer, window);
    //---pure boiler plate ---//

    /* per objectish */
   // createDescriptorSetLayout(renderer);
   // yk_create_gfx_pipeline(renderer);
    yk_cmd_innit(renderer);
    /* per objectish */
   // createDescriptorPool(renderer);
 
    //---can be optimized per object. But boilerplate for now --//
    yk_create_sync_objs(renderer);
    yk_desc_innit(renderer);
    pipeline_innit(renderer);
    //---can be optimized per object. But boilerplate for now --//

}

void yk_free_renderer(YkRenderer* renderer)
{
    vkDestroyPipelineLayout(renderer->device, renderer->triangle_pl_layout, 0);
    vkDestroyPipeline(renderer->device, renderer->triangle_pl, 0);

    vkDestroyPipelineLayout(renderer->device, renderer->gradient_pp_layouts,0);
    vkDestroyPipeline(renderer->device, renderer->gradient_pp,0);
    
    vkDestroyDescriptorPool(renderer->device,renderer->global_pool, 0);
    vkDestroyDescriptorSetLayout(renderer->device,renderer->draw_image_layouts,0);

    for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(renderer->device, renderer->frame_data[i].image_available_semawhore, 0);
        vkDestroySemaphore(renderer->device, renderer->frame_data[i].render_finished_semawhore, 0);
        vkDestroyFence(renderer->device, renderer->frame_data[i].in_flight_fence, 0);

        vkDestroyCommandPool(renderer->device, renderer->frame_data[i].cmd_pool, 0);
    }



  //  vkDestroyPipeline(renderer->device, renderer->r_pipeline, 0);
  //  vkDestroyPipelineLayout(renderer->device, renderer->r_pipeline_layout, 0);

    //Note(facts 11/24 0525): Glaring issue. max images is assumed to be three no matter what. Incase its lesser, I'll be deleting images that don't exist. Ugly bug.
    //Store image count somewhere. Good idea when you abstract the renderer further. But that is bikeshed until you understand vulkan and gfx programming.
    
    
    yk_cleanup_swapchain(renderer);


  //  vkDestroyDescriptorPool(renderer->device, renderer->descriptorPool, 0);
  //  vkDestroyDescriptorSetLayout(renderer->device, renderer->r_descriptorSetLayout, 0);

    vmaDestroyAllocator(renderer->vma_allocator);

    vkDestroyDevice(renderer->device, 0);
    vkDestroySurfaceKHR(renderer->vk_instance, renderer->surface, 0);

#if VK_USE_VALIDATION_LAYERS
    PFN_vkDestroyDebugUtilsMessengerEXT phunk = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(renderer->vk_instance, "vkDestroyDebugUtilsMessengerEXT");
    phunk(renderer->vk_instance, renderer->debug_messenger, 0);
#endif

    vkDestroyInstance(renderer->vk_instance, 0);
}


void transition_image(YkRenderer* renderer, VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 barrier = { };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.pNext = 0;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;

    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    barrier.oldLayout = currentLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;


    barrier.image = image;

    barrier.subresourceRange.aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkDependencyInfo dep_info = {};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &dep_info);
}


void yk_cmd_innit(YkRenderer* renderer)
{
    VkCommandPoolCreateInfo cmd_pool_info = { };
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = 0;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_info.queueFamilyIndex = Q_FAM_GFX;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResultAssert(vkCreateCommandPool(renderer->device, &cmd_pool_info, 0, &renderer->frame_data[i].cmd_pool), "Command pool creation");

        VkCommandBufferAllocateInfo vk_cmd_buffer_alloc_info = { };
        vk_cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        vk_cmd_buffer_alloc_info.pNext = 0;
        vk_cmd_buffer_alloc_info.commandPool = renderer->frame_data[i].cmd_pool;
        vk_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vk_cmd_buffer_alloc_info.commandBufferCount = 1;


        VkResultAssert(vkAllocateCommandBuffers(renderer->device, &vk_cmd_buffer_alloc_info, &renderer->frame_data[i].cmd_buffers), "Command Buffer allocation");
    }
}


u32 findMemoryType(YkRenderer* renderer, u32 typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(renderer->phys_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Assert(false, "Failed to find suitable memory type!");
    return 69420;
}


void yk_create_sync_objs(YkRenderer* renderer)
{
    //https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation

    //Some 7 stuff. I need semawhores

    //if semaphores aren't extended with semaphore types, they will be binary


    VkSemaphoreCreateInfo vk_semawhore_create_info = { };
    vk_semawhore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk_semawhore_create_info.pNext = 0;
    vk_semawhore_create_info.flags = 0;

    VkFenceCreateInfo vk_fence_create_info = { };
    vk_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResultAssert(vkCreateSemaphore(renderer->device, &vk_semawhore_create_info, 0, &renderer->frame_data[i].image_available_semawhore), "Image ready semaphore");
        VkResultAssert(vkCreateSemaphore(renderer->device, &vk_semawhore_create_info, 0, &renderer->frame_data[i].render_finished_semawhore), "Render finished semaphore");
        VkResultAssert(vkCreateFence(renderer->device, &vk_fence_create_info, 0, &renderer->frame_data[i].in_flight_fence), "flight fence");
    }

}

void yk_renderer_draw(YkRenderer* renderer, YkWindow* win)
{
    yk_frame_data* current_frame = &renderer->frame_data[renderer->current_frame];

    VkResultAssert(vkWaitForFences(renderer->device, 1, &current_frame->in_flight_fence, VK_TRUE, UINT64_MAX), "Wait for fences")
    VkResultAssert(vkResetFences(renderer->device, 1, &current_frame->in_flight_fence), "Reset fences");

    u32 imageIndex = -1;

    if (vkAcquireNextImageKHR(renderer->device, renderer->swapchain, UINT64_MAX,
        current_frame->image_available_semawhore,
        VK_NULL_HANDLE, &imageIndex) == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if (yk_recreate_swapchain(renderer, win) == false)
        {
            return;
        }
    }

    VkCommandBufferBeginInfo vk_cmd_buffer_begin_info = yk_cmd_buffer_begin_info_create(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);   
    VkResultAssert(vkBeginCommandBuffer(current_frame->cmd_buffers, &vk_cmd_buffer_begin_info), "Command buffer begin");

    transition_image(renderer, current_frame->cmd_buffers, renderer->draw_image.image , VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    yk_renderer_draw_bg(renderer, current_frame->cmd_buffers);

    transition_image(renderer, current_frame->cmd_buffers, renderer->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    yk_renderer_draw_triangle(renderer, current_frame->cmd_buffers);

    transition_image(renderer, current_frame->cmd_buffers, renderer->draw_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transition_image(renderer, current_frame->cmd_buffers, renderer->sc_images[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copy_image_to_image(current_frame->cmd_buffers, renderer->draw_image.image,
                        renderer->sc_images[imageIndex], VkExtent2D{ renderer->draw_image.imageExtent.width, renderer->draw_image.imageExtent.height },
                        renderer->sc_extent);


    transition_image(renderer, current_frame->cmd_buffers, renderer->sc_images[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkResultAssert(vkEndCommandBuffer(current_frame->cmd_buffers), "Command buffer end");

    VkSemaphoreSubmitInfo signal_semaphore = yk_semawhore_submit_info_create(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, current_frame->render_finished_semawhore);
    VkSemaphoreSubmitInfo wait_semaphore = yk_semawhore_submit_info_create(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, current_frame->image_available_semawhore);

    VkCommandBufferSubmitInfo cmd_buffer_submit_info = yk_cmd_buffer_submit_info_create(current_frame->cmd_buffers);

    VkSubmitInfo2 submit_info = yk_submit_info_create(&cmd_buffer_submit_info, &signal_semaphore, &wait_semaphore);



    VkResultAssert(vkQueueSubmit2(renderer->gfx_q, 1, &submit_info, current_frame->in_flight_fence), "Draw command buffer submitted");


    VkPresentInfoKHR vk_present_info = { };
    vk_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    vk_present_info.waitSemaphoreCount = 1;
    vk_present_info.pWaitSemaphores = &current_frame->render_finished_semawhore;

    vk_present_info.swapchainCount = 1;
    vk_present_info.pSwapchains = &renderer->swapchain;
    vk_present_info.pImageIndices = &imageIndex;

    vk_present_info.pResults = 0;

    //present q same as graphics for now
    VkResult qpresent_result = vkQueuePresentKHR(renderer->gfx_q, &vk_present_info);

    if (qpresent_result == VK_ERROR_OUT_OF_DATE_KHR || qpresent_result == VK_SUBOPTIMAL_KHR || win->win_data.is_resized)
    {
        win->win_data.is_resized = false;
        if (yk_recreate_swapchain(renderer, win) == false)
        {
            return;
        }
    }

    renderer->current_frame = (renderer->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    renderer->frames_rendered++;
}

void yk_renderer_wait(YkRenderer* renderer)
{
    vkDeviceWaitIdle(renderer->device);
}

b8 yk_recreate_swapchain(YkRenderer* renderer, YkWindow* win)
{
    if (!win->win_data.is_running)
    {
        return false;
    }

    vkDeviceWaitIdle(renderer->device);

    yk_cleanup_swapchain(renderer);



    yk_create_swapchain(renderer, win);

    return true;
}

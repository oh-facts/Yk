#include <renderer/renderer.h>


#if defined (_WIN32)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

void _print_device_details(VkPhysicalDeviceProperties* vk_phys_device_props)
{
    printf("\nSelected Gpu\n");
    printf("----------\n");
    printf("%s\n%u\n%s\n", vk_phys_device_props->deviceName, vk_phys_device_props->driverVersion, vk_phys_device_props->deviceType == 2 ? "Discrete GPU" : "Integreted probably");
    printf("----------\n");
}

#if LOG_DEVICE_DETAILS
#define log_device(Expression) _print_device_details(Expression);
#else
#define log_device(Expession)
#endif

//ToDo(facts): Use transient memory instead of allocating it

void check_device_extension_support(VkPhysicalDevice device)
{

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    printf("Vulkan Device Available Extentions ");
    for (uint32_t j = 0; j < extensionCount; ++j)
    {

        printf("%s\n", availableExtensions[j].extensionName);

    }
    printf("\n");

    free(availableExtensions);
}

void check_instance_extension_support()
{

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);

    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("Vulkan Instance Available Extensions:\n");
    for (uint32_t i = 0; i < extensionCount; ++i)
    {
        printf("%-100s\n", extensions[i].extensionName);
    }
    printf("\n");

    free(extensions);
}

//ToDo(facts): Strict mode, where anything apart from success is a crash
void _check_vk_result(VkResult result, const char* msg) {

    if (result == VK_SUCCESS)
    {
#if VK_PRINT_SUCCESS

        const int max_len = 30;
        const int gap = 5;

        printf("%*s%*sis great success\n", max_len, msg, gap, "");

#endif

        return;
    }

    const char* error_msg = string_VkResult(result);
    printf("%s failed %s\n", msg, error_msg);

    if (result == VK_INCOMPLETE || result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    else
    {
        volatile int* ptr = 0;
        *ptr = 0;
    }

}

#if VkDEBUG
#define VkResultAssert(result_expr, msg_expr ) _check_vk_result(result_expr, msg_expr);
#else
#define VkResultAssert(result_expr, msg_expr ) result_expr; 
#endif

#if VK_EXT_PRINT_DEBUG
#define log_extention(Expression) Expression;
#else
#define log_extention(Expression)
#endif

void yk_get_framebuffer_size(YkWindow* win, u32* width, u32* height)
{
    RECT clientRect;
    GetClientRect((HWND)win->win_handle, &clientRect);

    *width = (u32)clientRect.right - clientRect.left;
    *height = (u32)clientRect.bottom - clientRect.top;
}


/*
    Pure vulkan related boilerplate
*/
void yk_innit_vulkan(YkRenderer* renderer);
void yk_create_surface(YkRenderer* renderer, void* native_handle);
void yk_pick_physdevice(YkRenderer* renderer);
void yk_find_queues(YkRenderer* renderer);
void yk_create_device(YkRenderer* renderer);
void yk_create_swapchain(YkRenderer* renderer, YkWindow* win);
/*
    -------------------------------
*/

//void createDescriptorSetLayout(YkRenderer* renderer);
//void yk_create_gfx_pipeline(YkRenderer* renderer);

void yk_cmd_innit(YkRenderer* renderer);

void yk_create_vert_buffer(YkRenderer* renderer, const vertex vertices[], VkDeviceSize bufferSize, buffer* vert_buffer);
void yk_create_index_buffer(YkRenderer* renderer, const u16 indices[], VkDeviceSize bufferSize, buffer* index_buffer);

//void createDescriptorPool(YkRenderer* renderer);

//void createUniformBuffers(YkRenderer* renderer, VkDeviceSize bufferSize, ubuffer ubo[]);
//void updateUniformBuffer(YkRenderer* renderer, ubuffer ubo[], uint32_t currentImage, int flag);
//void createDescriptorSets(YkRenderer* renderer, ubuffer* ubo, render_object* ro);

void yk_create_sync_objs(YkRenderer* renderer);
b8 yk_recreate_swapchain(YkRenderer* renderer, YkWindow* win);
void copyBuffer(YkRenderer* renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void yk_create_buffer(YkRenderer* ren, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

void yk_cleanup_swapchain(YkRenderer* renderer);

void pipeline_innit(YkRenderer* renderer);
void gradient_pipeline(YkRenderer* renderer);
/*
 -------util-------
*/
VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = 0;
    
    info.imageType = VK_IMAGE_TYPE_2D;
    
    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    info.samples = VK_SAMPLE_COUNT_1_BIT;

    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usage_flags;
    
    return info;
}

VkImageViewCreateInfo image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags)
{
    VkImageViewCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = 0;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspect_flags;

    return info;
}

void copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size)
{
    
    VkImageBlit2 blit_reg = {};
    blit_reg.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

    //srcOffset[0] is top left corner. [1] is bottom right. Over here we are specifying the extent of the copy, which is whole image.
    blit_reg.srcOffsets[1].x = src_size.width;
    blit_reg.srcOffsets[1].y = src_size.height;
    blit_reg.srcOffsets[1].z = 1;

    blit_reg.dstOffsets[1].x = dst_size.width;
    blit_reg.dstOffsets[1].y = dst_size.height;
    blit_reg.dstOffsets[1].z = 1;

    blit_reg.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_reg.srcSubresource.baseArrayLayer = 0;
    blit_reg.srcSubresource.layerCount= 1;
    blit_reg.srcSubresource.mipLevel = 0;

    blit_reg.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_reg.dstSubresource.baseArrayLayer = 0;
    blit_reg.dstSubresource.layerCount = 1;
    blit_reg.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blit_info = {};
    blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    blit_info.dstImage = dst;
    blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blit_info.srcImage = src;
    blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blit_info.filter = VK_FILTER_LINEAR;
    blit_info.regionCount = 1;
    blit_info.pRegions = &blit_reg;

    vkCmdBlitImage2(cmd, &blit_info);
}

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

void yk_destroy_model(YkRenderer* renderer, render_object* render_object)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(renderer->device, render_object->ubo[i].buffer.handle, 0);
        vkFreeMemory(renderer->device, render_object->ubo[i].buffer.memory, 0);
    }

    vkDestroyBuffer(renderer->device, render_object->index_buffer.handle, 0);
    vkFreeMemory(renderer->device, render_object->index_buffer.memory, 0);

    vkDestroyBuffer(renderer->device, render_object->vert_buffer.handle, 0);
    vkFreeMemory(renderer->device, render_object->vert_buffer.memory, 0);
}

void  yk_innit_vulkan(YkRenderer* renderer)
{
    log_extention(check_instance_extension_support())

    VkApplicationInfo vk_app_info = {};
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pNext = 0;
    vk_app_info.pApplicationName = "yekate";
    vk_app_info.applicationVersion = 0;
    vk_app_info.pEngineName = "yk";
    vk_app_info.engineVersion = 0;
    vk_app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo vk_create_info = { };
    vk_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_create_info.pNext = 0;
    vk_create_info.flags = 0;
    vk_create_info.pApplicationInfo = &vk_app_info;



#if VK_USE_VALIDATION_LAYERS

#define VALIDATION_LAYERS_NUM 1

    const char* validation_layers[VALIDATION_LAYERS_NUM] = { };
    validation_layers[0] = "VK_LAYER_KHRONOS_validation";

    //validation layer support check


    vk_create_info.enabledLayerCount = VALIDATION_LAYERS_NUM;
    vk_create_info.ppEnabledLayerNames = validation_layers;

#endif

    /*
        When they adding constexpr to C fr fr
    */
#if VK_USE_VALIDATION_LAYERS
#define num_extensions 3
#else
#define num_extensions 2
#endif

    const char* enabled_extensions[num_extensions] = { };
    enabled_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;

#if defined(_WIN32)
    enabled_extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(__ANDROID__)
    enabled_extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif defined(__linux__)
    enabled_extensions[1] = (VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

#if VK_USE_VALIDATION_LAYERS
    enabled_extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif


    vk_create_info.enabledExtensionCount = num_extensions;
    vk_create_info.ppEnabledExtensionNames = enabled_extensions;


    VkResultAssert(vkCreateInstance(&vk_create_info, 0, &renderer->vk_instance), "Vulkan instance creation")

        //Debug messenger
#if VK_USE_VALIDATION_LAYERS

        VkDebugUtilsMessengerCreateInfoEXT vk_debug_messenger_create_info = { };
    vk_debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    vk_debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    vk_debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vk_debug_messenger_create_info.pfnUserCallback = debugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(renderer->vk_instance, "vkCreateDebugUtilsMessengerEXT");
    VkResultAssert(vkCreateDebugUtilsMessengerEXT(renderer->vk_instance, &vk_debug_messenger_create_info, 0, &renderer->debug_messenger), "Debug messenger");

#endif


}

void yk_create_surface(YkRenderer* renderer, void * native_handle)
{
    //Needs to be done first because queues need to be able to present and for that I need a surface
    //34.2.3
    //Win 32 surface platform

    VkWin32SurfaceCreateInfoKHR vk_win32_surface_create_info_khr = {  };
    vk_win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vk_win32_surface_create_info_khr.pNext = 0;
    vk_win32_surface_create_info_khr.flags = 0;
    vk_win32_surface_create_info_khr.hinstance = GetModuleHandle(0);
    vk_win32_surface_create_info_khr.hwnd = (HWND)native_handle;

    VkResultAssert(vkCreateWin32SurfaceKHR(renderer->vk_instance, &vk_win32_surface_create_info_khr, 0, &renderer->surface), "Win 32 Surface Creation");
}

void yk_pick_physdevice(YkRenderer* renderer)
{
    // 5.1 starts here
    //Physical Device

    constexpr i32 max_devices = 3;

    u32 devices = 0;
    vkEnumeratePhysicalDevices(renderer->vk_instance, &devices, 0);

    Assert(devices <= max_devices, "More than 3 graphics cards? Wth?")

        VkPhysicalDevice device_list[max_devices] = { };

    VkResultAssert(vkEnumeratePhysicalDevices(renderer->vk_instance, &devices, device_list), "physical device detection")

        //ToDo(facts): Account for multiple "good" gpus (one gpu is best)

        //not compulsory, but appreciated
        bool is_discrete[max_devices] = { false };

    //compulsory features are dynamic rendering, syncronization2, buffer device address and descriptor indexing
    bool compulsory_features[max_devices] = { false };

    for (i32 i = 0; i < devices; i++)
    {
        VkPhysicalDeviceProperties vk_phys_device_props = { };
        vkGetPhysicalDeviceProperties(device_list[i], &vk_phys_device_props);

        VkPhysicalDeviceFeatures2 vk_phys_device_feat = { };
        vk_phys_device_feat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        VkPhysicalDeviceVulkan13Features vk13_feat = { };
        VkPhysicalDeviceVulkan12Features vk12_feat = { };

        vk12_feat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vk13_feat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        vk_phys_device_feat.pNext = &vk12_feat;
        vk12_feat.pNext = &vk13_feat;
        vk13_feat.pNext = 0;

        vkGetPhysicalDeviceFeatures2(device_list[i], &vk_phys_device_feat);


        if (vk_phys_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            is_discrete[i] = true;
        }

        if (vk13_feat.dynamicRendering && vk13_feat.synchronization2 && vk12_feat.bufferDeviceAddress && vk12_feat.descriptorIndexing)
        {
            compulsory_features[i] = true;
        }

        if (is_discrete[i] && compulsory_features[i])
        {
            renderer->phys_device = device_list[i];
            return;
        }
    }

    for (i32 i = 0; i < 3; i++)
    {
        if (compulsory_features[i])
        {
            renderer->phys_device = device_list[i];
            printf("You don't have a discrete gpu. But your gpu supports the features required");
            return;
        }
    }

    //Control shouldn't come here
    Assert(false, "Your gpu is trash lmao. It doesn't have basic (vulkan 1.2 and 1.3) features.");

    //log_extention(check_device_extension_support(renderer->phys_device))
}


void yk_create_device(YkRenderer* renderer)
{

    //Logical Device starts here
    float queue_priority = 1;

    // Note(facts): The graphic , compute and present queue families are all index 0. And this is common behaviour. For now I am leaving this
    // like this. Later, I will make sure that incase they are different families, each gets its own queue

    VkDeviceQueueCreateInfo vk_device_q_create_info = { };
    vk_device_q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_device_q_create_info.pNext = 0;
    vk_device_q_create_info.flags = 0;
    vk_device_q_create_info.queueFamilyIndex = Q_FAM_GFX;
    //this is number of queues you want to create. Not how many queues are available in that queue family
    vk_device_q_create_info.queueCount = 1;
    vk_device_q_create_info.pQueuePriorities = &queue_priority;


    //ToDo(facts 1032 1/4/24): When selecting necessary features in pick physical device, also enable them there.
    //                         And maybe make a neater way to enable features.
    VkPhysicalDeviceDynamicRenderingFeaturesKHR vk_dynamic_rendering_feature = { };
    vk_dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    vk_dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceSynchronization2FeaturesKHR vk_sync2_feet = {};
    vk_sync2_feet.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    vk_sync2_feet.synchronization2 = VK_TRUE;

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR vk_buffer_device_address = {};
    vk_buffer_device_address.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
    vk_buffer_device_address.bufferDeviceAddress = VK_TRUE;
    
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT vk_desc_indexing = {};
    vk_desc_indexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    vk_desc_indexing.descriptorBindingPartiallyBound = VK_TRUE;


    vk_buffer_device_address.pNext = &vk_desc_indexing;

    vk_sync2_feet.pNext = &vk_buffer_device_address;

    vk_dynamic_rendering_feature.pNext = &vk_sync2_feet;

    const char* device_extention_names[2] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

    VkDeviceCreateInfo vk_device_create_info = { };
    vk_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vk_device_create_info.pNext = &vk_dynamic_rendering_feature;
    vk_device_create_info.flags = 0;
    vk_device_create_info.queueCreateInfoCount = 1;
    vk_device_create_info.pQueueCreateInfos = &vk_device_q_create_info;
    vk_device_create_info.enabledLayerCount = 0;    //ignored
    vk_device_create_info.ppEnabledLayerNames = 0;  //ignored
    vk_device_create_info.enabledExtensionCount = 2;
    vk_device_create_info.ppEnabledExtensionNames = device_extention_names;
    vk_device_create_info.pEnabledFeatures = 0;

    VkResultAssert(vkCreateDevice(renderer->phys_device, &vk_device_create_info, 0, &renderer->device), "Vulkan device creation");

}


void yk_find_queues(YkRenderer* renderer)
{
    //Nvidia 4090 has 5. I only intend to use 3. 99% chance are they all refer to the same queue.
#define max_queues 5
    u32 queues = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(renderer->phys_device, &queues, 0);

    Assert(queues <= max_queues, "More queues found than supported")
        //w
        VkQueueFamilyProperties vk_q_fam_prop_list[max_queues];
    u32 current_queues = max_queues;
    vkGetPhysicalDeviceQueueFamilyProperties(renderer->phys_device, &current_queues, vk_q_fam_prop_list);

    i32 present_queue_found = -1;
    for (i32 i = 0; i < current_queues; i++)
    {
        VkQueueFlags qflags = vk_q_fam_prop_list[i].queueFlags;
        if (qflags & VK_QUEUE_GRAPHICS_BIT)
        {
            renderer->qfams[Q_FAM_GFX] = i;
        }

        if ((qflags & VK_QUEUE_GRAPHICS_BIT) && (qflags & VK_QUEUE_COMPUTE_BIT))
        {
            renderer->qfams[Q_FAM_GFX_COMPUTE] = i;
        }

        if (present_queue_found == -1)
        {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(renderer->phys_device, i, renderer->surface, &present_support);

            if (present_support == VK_TRUE)
            {
                renderer->qfams[Q_FAM_PRESENT] = i;
            }

            present_queue_found = 1;

        }

    }


    Assert(renderer->qfams[Q_FAM_GFX] != -1, "Graphics Queue not found")
    Assert(renderer->qfams[Q_FAM_GFX_COMPUTE] != -1, "Graphics Compute Queue not found")
    Assert(renderer->qfams[Q_FAM_PRESENT] != -1, "Present Queue not found")


    // ToDo(facts 12/24 0439): Its all one queue. Come back to this later. If even one gfx card is made where the queues are literally different,
    // I will account for them
    vkGetDeviceQueue(renderer->device, Q_FAM_GFX, 0, &renderer->gfx_q);
}

void yk_create_swapchain(YkRenderer* renderer, YkWindow* win)
{
    //https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html

    VkSurfaceCapabilitiesKHR vk_surface_caps = { };
    VkResultAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->phys_device, renderer->surface, &vk_surface_caps), "Surface Capabilities poll");

    Assert(vk_surface_caps.maxImageCount >= 1, "Max images supported than 1");
    uint32_t imageCount = vk_surface_caps.minImageCount + 1;
    if (imageCount > vk_surface_caps.maxImageCount)
        imageCount = vk_surface_caps.maxImageCount;

    VkExtent2D vk_extent = { };
    if (vk_surface_caps.currentExtent.width != UINT32_MAX) {
        vk_extent = vk_surface_caps.currentExtent;
    }
    else {
        u32 width = 0;
        u32 height = 0;
        yk_get_framebuffer_size(win, &width, &height);

        VkExtent2D actualExtent = {
            .width = width,
            .height = height
        };

        // Ensure the width and height are never zero
        actualExtent.width = (actualExtent.width == 0) ? 1 : actualExtent.width;
        actualExtent.height = (actualExtent.height == 0) ? 1 : actualExtent.height;

        actualExtent.width = CLAMP(actualExtent.width, vk_surface_caps.minImageExtent.width, vk_surface_caps.maxImageExtent.width);
        actualExtent.height = CLAMP(actualExtent.height, vk_surface_caps.minImageExtent.height, vk_surface_caps.maxImageExtent.height);

        vk_extent = actualExtent;
    }


    //cursed fuckery
    renderer->sc_extent = vk_extent;

    VkViewport vk_viewport = { };
    vk_viewport.x = 0.0f;
    vk_viewport.y = 0.0f;
    vk_viewport.width = (f32)renderer->sc_extent.width;
    vk_viewport.height = (f32)renderer->sc_extent.height;
    vk_viewport.minDepth = 0.0f;
    vk_viewport.maxDepth = 1.0f;

    VkRect2D vk_scissor = { };
    vk_scissor.offset = VkOffset2D{ 0,0 };
    vk_scissor.extent = renderer->sc_extent;

    renderer->scissor = vk_scissor;
    renderer->viewport = vk_viewport;


    //ToDo(facts): Start doing this on the heap

#define max_format_count 5
    u32 vk_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->phys_device, renderer->surface, &vk_format_count, 0);
    Assert(vk_format_count > 0, "Format count less than 1")
        Assert(vk_format_count <= max_format_count, "Too many formats")

        VkSurfaceFormatKHR vk_surface_format_list[max_format_count] = { };
    VkResultAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->phys_device, renderer->surface, &vk_format_count, vk_surface_format_list), "Surface formats obtain")

        //ToDo(facts, 12/22): Stop being a smartass at 5:58am. Go to sleep
        VkSurfaceFormatKHR surface_format = { };
    for (i32 i = vk_format_count - 1; i >= 0; i--)
    {
        surface_format = vk_surface_format_list[i];
        if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            break;
        }
    }

#define max_present_mode 4
    u32 vk_present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->phys_device, renderer->surface, &vk_present_mode_count, 0);
    Assert(vk_present_mode_count > 0, "Less than 1 present modes found")
        Assert(vk_present_mode_count <= max_present_mode, "Too many present modes")

        VkPresentModeKHR vk_present_mode_list[max_present_mode] = { };

    VkResultAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->phys_device, renderer->surface, &vk_present_mode_count, vk_present_mode_list), "Device Present Modes")

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    //https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html
    for (u32 i = 0; i < vk_present_mode_count; i++)
    {
        if (vk_present_mode_list[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }

        if (vk_present_mode_list[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    //34.10
    //Swapchain helps to display rendering results to surface

    u32 vk_qfam_indices[1] = { };
    VkSwapchainCreateInfoKHR vk_swapchain_create_info = { };
    vk_swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vk_swapchain_create_info.pNext = 0;
    vk_swapchain_create_info.flags = 0;
    vk_swapchain_create_info.surface = renderer->surface;
    vk_swapchain_create_info.minImageCount = imageCount;
    vk_swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    vk_swapchain_create_info.imageFormat = surface_format.format;
    vk_swapchain_create_info.imageExtent = vk_extent;
    vk_swapchain_create_info.imageArrayLayers = 1;
    vk_swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vk_swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vk_swapchain_create_info.queueFamilyIndexCount = 1;
    vk_swapchain_create_info.pQueueFamilyIndices = vk_qfam_indices;
    vk_swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    vk_swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vk_swapchain_create_info.presentMode = present_mode;
    vk_swapchain_create_info.oldSwapchain = 0; //ToDo(facts): Get back to later
    vk_swapchain_create_info.clipped = VK_TRUE; //Note(facts): Read about later

    vkCreateSwapchainKHR(renderer->device, &vk_swapchain_create_info, 0, &renderer->swapchain);
    //   VkResultAssert(, "Created Swapchain");

#define max_images 3

    u32 vk_image_num = 0;
    vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &vk_image_num, 0);
    Assert(vk_image_num <= max_images, "More swapchain images than expected")

    VkResultAssert(vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &vk_image_num, renderer->sc_images), "Swapchain images found");




    for (i32 i = 0; i < vk_image_num; i++)
    {
        VkImageViewCreateInfo vk_image_view_create_info = { };
        vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vk_image_view_create_info.pNext = 0;
        vk_image_view_create_info.image = renderer->sc_images[i];
        vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vk_image_view_create_info.format = surface_format.format;

        VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_IDENTITY };
        vk_image_view_create_info.components = mapping;

        VkImageSubresourceRange subresourcerange = {
                                                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                    .baseMipLevel = 0,
                                                    .levelCount = 1  ,
                                                    .baseArrayLayer = 0,
                                                    .layerCount = 1
        };

        vk_image_view_create_info.subresourceRange = subresourcerange;
        char str[25];
        sprintf(str, "Image View Creation %d", i);
        VkResultAssert(vkCreateImageView(renderer->device, &vk_image_view_create_info, 0, &renderer->sc_image_views[i]), str);
    }

    VkExtent3D draw_image_extent = { (u32)win->win_data.size_x, (u32)win->win_data.size_y, 1 };

    renderer->draw_image.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    renderer->draw_image.imageExtent = draw_image_extent;

    VkImageUsageFlags draw_image_usage_flags = {};
    draw_image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    draw_image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    draw_image_usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    draw_image_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo draw_image_create_info = image_create_info(renderer->draw_image.imageFormat, draw_image_usage_flags, draw_image_extent);

    VmaAllocationCreateInfo draw_image_alloc_info = {};
    draw_image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    draw_image_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkResultAssert(vmaCreateImage(renderer->vma_allocator, &draw_image_create_info, &draw_image_alloc_info,
                                  &renderer->draw_image.image, &renderer->draw_image.allocation, 0), "Draw Image creation")

    VkImageViewCreateInfo draw_image_view_create_info = image_view_create_info(renderer->draw_image.imageFormat, renderer->draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VkResultAssert(vkCreateImageView(renderer->device, &draw_image_view_create_info, 0, &renderer->draw_image.imageView), "Draw image view creation");
    



}

void yk_cleanup_swapchain(YkRenderer* renderer)
{

    for (i32 i = 0; i < max_images; i++)
    {
        vkDestroyImageView(renderer->device, renderer->sc_image_views[i], 0);
    }

    //draw image
    vkDestroyImageView(renderer->device, renderer->draw_image.imageView, 0);
    vmaDestroyImage(renderer->vma_allocator, renderer->draw_image.image, renderer->draw_image.allocation);
    //draw image

    vkDestroySwapchainKHR(renderer->device, renderer->swapchain, 0);

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

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

void createDescriptorSetLayout(YkRenderer* renderer);
void yk_create_gfx_pipeline(YkRenderer* renderer);

void yk_cmd_innit(YkRenderer* renderer);

void yk_create_vert_buffer(YkRenderer* renderer, const vertex vertices[], VkDeviceSize bufferSize, buffer* vert_buffer);
void yk_create_index_buffer(YkRenderer* renderer, const u16 indices[], VkDeviceSize bufferSize, buffer* index_buffer);

void createDescriptorPool(YkRenderer* renderer);

void createUniformBuffers(YkRenderer* renderer, VkDeviceSize bufferSize, ubuffer ubo[]);
void updateUniformBuffer(YkRenderer* renderer, ubuffer ubo[], uint32_t currentImage, int flag);
void createDescriptorSets(YkRenderer* renderer, ubuffer* ubo, render_object* ro);

void yk_create_sync_objs(YkRenderer* renderer);
b8 yk_recreate_swapchain(YkRenderer* renderer, YkWindow* win);
void copyBuffer(YkRenderer* renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void yk_create_buffer(YkRenderer* ren, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

void yk_cleanup_swapchain(YkRenderer* renderer);


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
    blit_reg.srcOffsets[1].y = 1;

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
    createDescriptorSetLayout(renderer);
    yk_create_gfx_pipeline(renderer);
    yk_cmd_innit(renderer);
    /* per objectish */
    createDescriptorPool(renderer);
    //---can be optimized per object. But boilerplate for now --//
    yk_create_sync_objs(renderer);
    //---can be optimized per object. But boilerplate for now --//

}

void yk_free_renderer(YkRenderer* renderer)
{
    for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(renderer->device, renderer->frame_data[i].image_available_semawhore, 0);
        vkDestroySemaphore(renderer->device, renderer->frame_data[i].render_finished_semawhore, 0);
        vkDestroyFence(renderer->device, renderer->frame_data[i].in_flight_fence, 0);

        vkDestroyCommandPool(renderer->device, renderer->frame_data[i].cmd_pool, 0);
    }



    vkDestroyPipeline(renderer->device, renderer->gfx_pipeline, 0);
    vkDestroyPipelineLayout(renderer->device, renderer->pipeline_layout, 0);

    //Note(facts 11/24 0525): Glaring issue. max images is assumed to be three no matter what. Incase its lesser, I'll be deleting images that don't exist. Ugly bug.
    //Store image count somewhere. Good idea when you abstract the renderer further. But that is bikeshed until you understand vulkan and gfx programming.
    
    
    yk_cleanup_swapchain(renderer);


    vkDestroyDescriptorPool(renderer->device, renderer->descriptorPool, 0);
    vkDestroyDescriptorSetLayout(renderer->device, renderer->descriptorSetLayout, 0);

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
    renderer->extent = vk_extent;

    VkViewport vk_viewport = { };
    vk_viewport.x = 0.0f;
    vk_viewport.y = 0.0f;
    vk_viewport.width = (f32)renderer->extent.width;
    vk_viewport.height = (f32)renderer->extent.height;
    vk_viewport.minDepth = 0.0f;
    vk_viewport.maxDepth = 1.0f;

    VkRect2D vk_scissor = { };
    vk_scissor.offset = VkOffset2D{ 0,0 };
    vk_scissor.extent = renderer->extent;

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
    vk_swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

    VkResultAssert(vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &vk_image_num, renderer->swapchain_image_list), "Swapchain images found");




    for (i32 i = 0; i < vk_image_num; i++)
    {
        VkImageViewCreateInfo vk_image_view_create_info = { };
        vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vk_image_view_create_info.pNext = 0;
        vk_image_view_create_info.image = renderer->swapchain_image_list[i];
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
        VkResultAssert(vkCreateImageView(renderer->device, &vk_image_view_create_info, 0, &renderer->swapchain_image_view_list[i]), str);
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
        vkDestroyImageView(renderer->device, renderer->swapchain_image_view_list[i], 0);
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

void createDescriptorSetLayout(YkRenderer* renderer)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = { };
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = { };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkResultAssert(vkCreateDescriptorSetLayout(renderer->device, &layoutInfo, 0, &renderer->descriptorSetLayout), "descr set layout")
}

void yk_create_gfx_pipeline(YkRenderer* renderer)
{
    size_t vert_len = 0;
    size_t frag_len = 0;
    const char* vert_shader_code = yk_read_binary_file("res/vert.spv", &vert_len);
    const char* frag_shader_code = yk_read_binary_file("res/frag.spv", &frag_len);

    VkShaderModuleCreateInfo vk_vert_shader_module_create_info = { };
    vk_vert_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vk_vert_shader_module_create_info.codeSize = vert_len;
    vk_vert_shader_module_create_info.pCode = (u32*)vert_shader_code;

    VkShaderModule vk_vert_shader_module = { };
    VkResultAssert(vkCreateShaderModule(renderer->device, &vk_vert_shader_module_create_info, 0, &vk_vert_shader_module), "Vert Shader Module Creation");

    VkShaderModuleCreateInfo vk_frag_shader_module_create_info = { };
    vk_frag_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vk_frag_shader_module_create_info.codeSize = frag_len;
    vk_frag_shader_module_create_info.pCode = (u32*)frag_shader_code;

    VkShaderModule vk_frag_shader_module = { };
    VkResultAssert(vkCreateShaderModule(renderer->device, &vk_frag_shader_module_create_info, 0, &vk_frag_shader_module), "Frag Shader Module Creation");

    free((char*)vert_shader_code);
    free((char*)frag_shader_code);

    VkPipelineShaderStageCreateInfo vk_vert_shader_stage_info = { };
    vk_vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vk_vert_shader_stage_info.module = vk_vert_shader_module;
    vk_vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo vk_frag_shader_stage_info = { };
    vk_frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vk_frag_shader_stage_info.module = vk_frag_shader_module;
    vk_frag_shader_stage_info.pName = "main";


    VkPipelineShaderStageCreateInfo vk_shader_stages[] = { vk_vert_shader_stage_info, vk_frag_shader_stage_info };

    //Note(facts 12/23 0223) : Complete pipeline. Then rendering.

    VkDynamicState vk_dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo vk_dyn_state_create_info = { };
    vk_dyn_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    vk_dyn_state_create_info.dynamicStateCount = 2;
    vk_dyn_state_create_info.pDynamicStates = vk_dynamic_states;



    VkVertexInputBindingDescription binding_desc = vk_get_binding_desc();
    VkVertexInputAttributeDescription attrib_desc[2] = { };
    get_attrib_desc(attrib_desc);


    VkPipelineVertexInputStateCreateInfo vk_vertex_input_info = { };
    vk_vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vk_vertex_input_info.vertexBindingDescriptionCount = 1;
    vk_vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vk_vertex_input_info.vertexAttributeDescriptionCount = 2;
    vk_vertex_input_info.pVertexAttributeDescriptions = attrib_desc;

    VkPipelineInputAssemblyStateCreateInfo vk_input_asm = { };
    vk_input_asm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vk_input_asm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vk_input_asm.primitiveRestartEnable = VK_FALSE;

    VkViewport vk_viewport = { };
    vk_viewport.x = 0.0f;
    vk_viewport.y = 0.0f;
    vk_viewport.width = (f32)renderer->extent.width;
    vk_viewport.height = (f32)renderer->extent.height;
    vk_viewport.minDepth = 0.0f;
    vk_viewport.maxDepth = 1.0f;

    VkRect2D vk_scissor = { };
    vk_scissor.offset = VkOffset2D{ 0,0 };
    vk_scissor.extent = renderer->extent;

    VkPipelineViewportStateCreateInfo vk_viewport_state = { };
    vk_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vk_viewport_state.viewportCount = 1;
    vk_viewport_state.pViewports = &vk_viewport;
    vk_viewport_state.scissorCount = 1;
    vk_viewport_state.pScissors = &vk_scissor;

    VkPipelineRasterizationStateCreateInfo vk_rasterizer = { };
    vk_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vk_rasterizer.depthClampEnable = VK_FALSE;
    vk_rasterizer.rasterizerDiscardEnable = VK_FALSE;
    vk_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    vk_rasterizer.lineWidth = 1.0f;
    vk_rasterizer.cullMode = VK_CULL_MODE_NONE;
    vk_rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    //Note(facts 12/23 2:20): come back to this later
    vk_rasterizer.depthBiasEnable = VK_FALSE;
    vk_rasterizer.depthBiasConstantFactor = 0.0f;
    vk_rasterizer.depthBiasClamp = 0.0f;
    vk_rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo vk_multisampling = { };
    vk_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vk_multisampling.sampleShadingEnable = VK_FALSE;
    vk_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    vk_multisampling.minSampleShading = 1.0f;
    vk_multisampling.pSampleMask = 0;
    vk_multisampling.alphaToCoverageEnable = VK_FALSE;
    vk_multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState vk_color_blend_attatchment = { };
    vk_color_blend_attatchment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk_color_blend_attatchment.blendEnable = VK_FALSE;
    vk_color_blend_attatchment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    vk_color_blend_attatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    vk_color_blend_attatchment.colorBlendOp = VK_BLEND_OP_ADD;
    vk_color_blend_attatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    vk_color_blend_attatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    vk_color_blend_attatchment.alphaBlendOp = VK_BLEND_OP_ADD;
    vk_color_blend_attatchment.blendEnable = VK_TRUE;
    vk_color_blend_attatchment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    vk_color_blend_attatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    vk_color_blend_attatchment.colorBlendOp = VK_BLEND_OP_ADD;
    vk_color_blend_attatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    vk_color_blend_attatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    vk_color_blend_attatchment.alphaBlendOp = VK_BLEND_OP_ADD;

    //ToDo(facts): I have no fucking idea what this means
    //I just want a triangle
    VkPipelineColorBlendStateCreateInfo vk_color_blending = { };
    vk_color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vk_color_blending.logicOpEnable = VK_FALSE;
    vk_color_blending.logicOp = VK_LOGIC_OP_COPY;
    vk_color_blending.attachmentCount = 1;
    vk_color_blending.pAttachments = &vk_color_blend_attatchment;
    vk_color_blending.blendConstants[0] = 0.0f;
    vk_color_blending.blendConstants[1] = 0.0f;
    vk_color_blending.blendConstants[2] = 0.0f;
    vk_color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo vk_pipeline_layout_info = { };
    vk_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_pipeline_layout_info.setLayoutCount = 1;
    vk_pipeline_layout_info.pSetLayouts = &renderer->descriptorSetLayout;
    vk_pipeline_layout_info.pushConstantRangeCount = 0;
    vk_pipeline_layout_info.pPushConstantRanges = 0;

    VkPipelineLayout vk_pipeline_layout = { };

    VkResultAssert(vkCreatePipelineLayout(renderer->device, &vk_pipeline_layout_info, 0, &vk_pipeline_layout), "Pipeline layout creation");

    //graphics pipeline.
    //we want to do dynamic rendering
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info = { };
    vk_graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vk_graphics_pipeline_create_info.stageCount = 2;
    vk_graphics_pipeline_create_info.pStages = vk_shader_stages;
    vk_graphics_pipeline_create_info.pDynamicState = &vk_dyn_state_create_info;
    vk_graphics_pipeline_create_info.pVertexInputState = &vk_vertex_input_info;
    vk_graphics_pipeline_create_info.pInputAssemblyState = &vk_input_asm;
    vk_graphics_pipeline_create_info.pViewportState = &vk_viewport_state;
    vk_graphics_pipeline_create_info.pRasterizationState = &vk_rasterizer;
    vk_graphics_pipeline_create_info.pMultisampleState = &vk_multisampling;
    vk_graphics_pipeline_create_info.pColorBlendState = &vk_color_blending;
    vk_graphics_pipeline_create_info.layout = vk_pipeline_layout;
    vk_graphics_pipeline_create_info.renderPass = VK_NULL_HANDLE;


    VkResultAssert(vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1, &vk_graphics_pipeline_create_info, 0, &renderer->gfx_pipeline), "Graphics pipeline creation");

    vkDestroyShaderModule(renderer->device, vk_frag_shader_module, 0);
    vkDestroyShaderModule(renderer->device, vk_vert_shader_module, 0);

    //Tee hee
    renderer->pipeline_layout = vk_pipeline_layout;
    renderer->scissor = vk_scissor;
    renderer->viewport = vk_viewport;
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



void yk_create_vert_buffer(YkRenderer* renderer, const vertex vertices[], VkDeviceSize bufferSize, buffer* vert_buffer)
{//we

//    VkDeviceSize bufferSize = sizeof(vertices[0]) * 4;

    VkBuffer staging_buffer = { };
    VkDeviceMemory staging_buffer_memory = { };

    yk_create_buffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void* data;
    vkMapMemory(renderer->device, staging_buffer_memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);

    yk_create_buffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vert_buffer->handle, &vert_buffer->memory);

    copyBuffer(renderer, staging_buffer, vert_buffer->handle, bufferSize);

    vkDestroyBuffer(renderer->device, staging_buffer, 0);
    vkFreeMemory(renderer->device, staging_buffer_memory, 0);
}

void yk_create_index_buffer(YkRenderer* renderer, const u16 indices[], VkDeviceSize bufferSize, buffer* index_buffer)
{
    //   VkDeviceSize bufferSize = sizeof(indices[0]) * 6;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    yk_create_buffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(renderer->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(renderer->device, stagingBufferMemory);

    yk_create_buffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &index_buffer->handle, &index_buffer->memory);

    copyBuffer(renderer, stagingBuffer, index_buffer->handle, bufferSize);

    vkDestroyBuffer(renderer->device, stagingBuffer, 0);
    vkFreeMemory(renderer->device, stagingBufferMemory, 0);
}

void createUniformBuffers(YkRenderer* renderer, VkDeviceSize bufferSize, ubuffer ubo[])
{
    //VkDeviceSize bufferSize = sizeof(mvp_matrix);


    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        yk_create_buffer(renderer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &ubo[i].buffer.handle, &ubo[i].buffer.memory);

        vkMapMemory(renderer->device, ubo[i].buffer.memory, 0, bufferSize, 0, &ubo[i].mapped);
    }
}

void updateUniformBuffer(YkRenderer* renderer, ubuffer ubo[], uint32_t currentImage, int flag)
{

    clock_t current_time = clock();
    f32 time = (f32)(current_time - renderer->clock) / CLOCKS_PER_SEC;

    mvp_matrix mvp_mat = { };

    mvp_mat.model = yk_m4_identity();

    mvp_mat.model = yk_m4_translate(mvp_mat.model, v3{ 0.8f * flag, 0., -8. });

    mvp_mat.model = yk_m4_rotate(mvp_mat.model, time * 1.f, v3{ -5, 0, -999 });
    mvp_mat.view = yk_m4_look_at(v3{ 0, 0, 1 }, v3{ 0, 0, -1. }, v3{ 0, 1, 0 });
    mvp_mat.proj = yk_m4_perspective(DEG_TO_RAD * 45., renderer->extent.width / (f32)renderer->extent.height, 0.1f, 10.0f);

    // +z is back. +y is up , +x is right

    mvp_mat.proj.e[1][1] *= -1;

    memcpy(ubo[currentImage].mapped, &mvp_mat, sizeof(mvp_mat));
}


void createDescriptorPool(YkRenderer* renderer)
{
    VkDescriptorPoolSize poolSize = { };
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 4;

    VkDescriptorPoolCreateInfo poolInfo = { };
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = 4;

    VkResultAssert(vkCreateDescriptorPool(renderer->device, &poolInfo, 0, &renderer->descriptorPool), "descripter pool")

}

void createDescriptorSets(YkRenderer* renderer, ubuffer* ubo, render_object* ro)
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = { renderer->descriptorSetLayout,renderer->descriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer->descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    VkResultAssert(vkAllocateDescriptorSets(renderer->device, &allocInfo, &ro->descriptorSet[0]), "descriptor sets")

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo = { };
            bufferInfo.buffer = ubo->buffer.handle;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(mvp_matrix);

            VkWriteDescriptorSet descriptorWrite = { };
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = ro->descriptorSet[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(renderer->device, 1, &descriptorWrite, 0, 0);
        }
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

void yk_renderer_innit_model(YkRenderer* renderer, const vertex vertices[], const u16 indices[], render_object* render_object)
{

    yk_create_vert_buffer(renderer, vertices, sizeof(vertices[0]) * 4, &render_object->vert_buffer);
    yk_create_index_buffer(renderer, indices, sizeof(indices[0]) * 6, &render_object->index_buffer);


    createUniformBuffers(renderer, sizeof(mvp_matrix), render_object->ubo);

    createDescriptorSets(renderer, render_object->ubo, render_object);
}

/*
    Note: A draw loop should typically start with waiting for fences (fence ensure gpu / cpu sync)

    image index vs current frame
    https://stackoverflow.com/questions/72794542/c-vulkan-swapchain-image-index-vs-current-frame

    I still don't understand it. But basically, if you're dealing with swapchain index, use image index
*/

void yk_renderer_raster_draw(YkRenderer* renderer, YkWindow* win)
{
    yk_frame_data* current_frame = &renderer->frame_data[renderer->current_frame];

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(renderer->device, "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(renderer->device, "vkCmdEndRenderingKHR");


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



    VkResultAssert(vkResetCommandBuffer(current_frame->cmd_buffers, 0), "Cmd buffer reset");

    //command buffer record


    VkRenderingAttachmentInfoKHR vk_color_attachment = { };
    vk_color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_color_attachment.imageView = renderer->swapchain_image_view_list[imageIndex];
    vk_color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vk_color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vk_color_attachment.clearValue.color = VkClearColorValue{ 0.0f, 0.01f, 0.03f, 1.0f };

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

    VkCommandBufferBeginInfo vk_cmd_buffer_begin_info = yk_cmd_buffer_begin_info_create(0);
    VkResultAssert(vkBeginCommandBuffer(current_frame->cmd_buffers, &vk_cmd_buffer_begin_info), "Command buffer begin");
    //transition_image(renderer, current_frame->cmd_buffers, renderer->swapchain_image_list[renderer->current_frame], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);


    // Begin rendering
    vkCmdBeginRenderingKHR(current_frame->cmd_buffers, &vk_rendering_info);

    vkCmdBindPipeline(current_frame->cmd_buffers, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->gfx_pipeline);


    vkCmdSetViewport(current_frame->cmd_buffers, 0, 1, &renderer->viewport);
    vkCmdSetScissor(current_frame->cmd_buffers, 0, 1, &renderer->scissor);

    for (int i = 0; i < renderer->num_ro; i++)
    {

        VkBuffer vertex_buffers[] = { renderer->render_objects[i].vert_buffer.handle };

        //if you pass mutiple buffers
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(current_frame->cmd_buffers, 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(current_frame->cmd_buffers, renderer->render_objects[i].index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

        if (i % 2 == 0)
        {
            updateUniformBuffer(renderer, renderer->render_objects[i].ubo, renderer->current_frame, -1);
        }
        else
        {
            updateUniformBuffer(renderer, renderer->render_objects[i].ubo, renderer->current_frame, 1);
        }


        vkCmdBindDescriptorSets(current_frame->cmd_buffers, VK_PIPELINE_BIND_POINT_GRAPHICS,
            renderer->pipeline_layout, 0, 1, &renderer->render_objects[i].descriptorSet[renderer->current_frame], 0, 0);

        vkCmdDrawIndexed(current_frame->cmd_buffers, 6, 1, 0, 0, 0);

    }

    // End rendering
    vkCmdEndRenderingKHR(current_frame->cmd_buffers);

    transition_image(renderer, current_frame->cmd_buffers, renderer->swapchain_image_list[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkResultAssert(vkEndCommandBuffer(current_frame->cmd_buffers), "Command buffer end");

    //command buffer recording over

    VkPipelineStageFlags2 flags2 = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo signal_semaphore = yk_semawhore_submit_info_create(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, current_frame->render_finished_semawhore);
    VkSemaphoreSubmitInfo wait_semaphore = yk_semawhore_submit_info_create(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, current_frame->image_available_semawhore );

    VkCommandBufferSubmitInfo cmd_buffer_submit_info = yk_cmd_buffer_submit_info_create(current_frame->cmd_buffers);

    VkSubmitInfo2 submit_info = yk_submit_info_create(&cmd_buffer_submit_info, &signal_semaphore, &wait_semaphore);

    

    VkResultAssert(vkQueueSubmit2(renderer->gfx_q, 1, &submit_info, current_frame->in_flight_fence) , "Draw command buffer submitted");


    VkPresentInfoKHR vk_present_info = { };
    vk_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    vk_present_info.waitSemaphoreCount = 1;
    vk_present_info.pWaitSemaphores = &current_frame->render_finished_semawhore;

    vk_present_info.swapchainCount = 1;
    vk_present_info.pSwapchains = &renderer->swapchain;
    vk_present_info.pImageIndices = & imageIndex;

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
    // printf("we");
}

void yk_renderer_draw(YkRenderer* renderer, YkWindow* win)
{
    yk_frame_data* current_frame = &renderer->frame_data[renderer->current_frame];

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(renderer->device, "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(renderer->device, "vkCmdEndRenderingKHR");


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

    transition_image(renderer, current_frame->cmd_buffers, renderer->swapchain_image_list[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue;
    float flash = abs(sin(renderer->current_frame / 120.f));
    clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

    VkImageSubresourceRange clear_range = {
                                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                   .baseMipLevel = 0,
                                                   .levelCount = VK_REMAINING_MIP_LEVELS,
                                                   .baseArrayLayer = 0,
                                                   .layerCount = VK_REMAINING_ARRAY_LAYERS
    };

    vkCmdClearColorImage(current_frame->cmd_buffers, renderer->swapchain_image_list[imageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clear_range);

    transition_image(renderer, current_frame->cmd_buffers, renderer->swapchain_image_list[imageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    VkResultAssert(vkEndCommandBuffer(current_frame->cmd_buffers), "Command buffer end");

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

VkVertexInputBindingDescription vk_get_binding_desc()
{
    VkVertexInputBindingDescription bindingDescription = { };

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

void yk_create_buffer(YkRenderer* ren, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
    VkBufferCreateInfo bufferInfo = { };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResultAssert(vkCreateBuffer(ren->device, &bufferInfo, 0, buffer), "Created buffer")

        VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ren->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(ren, memRequirements.memoryTypeBits, properties);

    VkResultAssert(vkAllocateMemory(ren->device, &allocInfo, 0, bufferMemory), "Buffer memory allocation");

    vkBindBufferMemory(ren->device, *buffer, *bufferMemory, 0);

}

void copyBuffer(YkRenderer* renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer->frame_data[renderer->current_frame].cmd_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderer->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = { };
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(renderer->gfx_q, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderer->gfx_q);

    vkFreeCommandBuffers(renderer->device, renderer->frame_data[renderer->current_frame].cmd_pool, 1, &commandBuffer);

}
#include <renderer/ykr_instance.h>
#include <renderer/ykr_debug_util.h>


#if defined (_WIN32)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif



#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

void yk_get_framebuffer_size(YkWindow* win, u32* width, u32* height)
{
    RECT clientRect;
    GetClientRect((HWND)win->win_handle, &clientRect);

    *width = (u32)clientRect.right - clientRect.left;
    *height = (u32)clientRect.bottom - clientRect.top;
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
#define max_queues 7
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

    //swap chain images

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

    //draw image

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
    
    //depth image
    renderer->depth_image.imageFormat = VK_FORMAT_D32_SFLOAT;
    renderer->depth_image.imageExtent = draw_image_extent;

    VkImageUsageFlags depth_image_usage_flags = {};
    depth_image_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    
    VkImageCreateInfo depth_image_create_info = image_create_info(renderer->depth_image.imageFormat, depth_image_usage_flags, draw_image_extent);
    vmaCreateImage(renderer->vma_allocator, &depth_image_create_info, &draw_image_alloc_info, &renderer->depth_image.image, &renderer->depth_image.allocation, 0);

    VkImageViewCreateInfo dview_info = image_view_create_info(renderer->depth_image.imageFormat, renderer->depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkResultAssert(vkCreateImageView(renderer->device, &dview_info, 0, &renderer->depth_image.imageView), "depth image view creation");

}

void yk_cleanup_swapchain(YkRenderer* renderer)
{
    //sc image views
    for (i32 i = 0; i < max_images; i++)
    {
        vkDestroyImageView(renderer->device, renderer->sc_image_views[i], 0);
    }
    //-------------
    
    //draw image
    vkDestroyImageView(renderer->device, renderer->draw_image.imageView, 0);
    vmaDestroyImage(renderer->vma_allocator, renderer->draw_image.image, renderer->draw_image.allocation);
    //-------------

    //depth image
    vkDestroyImageView(renderer->device, renderer->depth_image.imageView, 0);
    vmaDestroyImage(renderer->vma_allocator, renderer->depth_image.image, renderer->depth_image.allocation);
    //-------------

    vkDestroySwapchainKHR(renderer->device, renderer->swapchain, 0);

}

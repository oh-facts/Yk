#include <renderer/context.h>

#if defined (_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif


void mn_context_innit(mn_context* context , void* surface_context)
{
    // vulkan instance init

    log_extention(check_instance_extension_support())

    VkApplicationInfo vk_app_info = { 0 };
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pNext = 0;
    vk_app_info.pApplicationName = "yekate";
    vk_app_info.applicationVersion = 0;
    vk_app_info.pEngineName = "yk";
    vk_app_info.engineVersion = 0;
    vk_app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo vk_create_info = { 0 };
    vk_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_create_info.pNext = 0;
    vk_create_info.flags = 0;
    vk_create_info.pApplicationInfo = &vk_app_info;



#if VK_USE_VALIDATION_LAYERS

#define VALIDATION_LAYERS_NUM 1

    const char* validation_layers[VALIDATION_LAYERS_NUM] = { 0 };
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

    const char* enabled_extensions[num_extensions] = { 0 };
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


    VkResultAssert(vkCreateInstance(&vk_create_info, 0, &context->vk_instance), "Vulkan instance creation")

        //Debug messenger
#if VK_USE_VALIDATION_LAYERS

    VkDebugUtilsMessengerCreateInfoEXT vk_debug_messenger_create_info = { 0 };
    vk_debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    vk_debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    vk_debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vk_debug_messenger_create_info.pfnUserCallback = debugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->vk_instance, "vkCreateDebugUtilsMessengerEXT");
    VkResultAssert(vkCreateDebugUtilsMessengerEXT(context->vk_instance, &vk_debug_messenger_create_info, 0, &context->debug_messenger), "Debug messenger");

#endif


    VkWin32SurfaceCreateInfoKHR vk_win32_surface_create_info_khr = { 0 };
    vk_win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vk_win32_surface_create_info_khr.pNext = 0;
    vk_win32_surface_create_info_khr.flags = 0;
    
#if defined(_WIN32)
    YkWindow* win32_win = (YkWindow*)surface_context;
    vk_win32_surface_create_info_khr.hinstance = win32_win->hinstance;
    vk_win32_surface_create_info_khr.hwnd = win32_win->win_handle;
    VkResultAssert(vkCreateWin32SurfaceKHR(context->vk_instance, &vk_win32_surface_create_info_khr, 0, &context->surface), "Win 32 Surface Creation");
#endif // DEBUG


}

void mn_context_free(mn_context* context)
{
#if VK_USE_VALIDATION_LAYERS
    PFN_vkDestroyDebugUtilsMessengerEXT phunk = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->vk_instance, "vkDestroyDebugUtilsMessengerEXT");
    phunk(context->vk_instance, context->debug_messenger, 0);
#endif

    vkDestroyInstance(context->vk_instance, 0);
}

void mn_device_innit(mn_context* context, mn_device* device)
{
    //setup device
    VkPhysicalDevice vk_phys_device = { 0 };

    #define max_devices 3

    u32 devices = 0;
    vkEnumeratePhysicalDevices(context->vk_instance, &devices, 0);

    Assert(devices <= max_devices, "More than 3 graphics cards? Wth?")

    VkPhysicalDevice device_list[max_devices] = { 0 };

    VkResultAssert(vkEnumeratePhysicalDevices(context->vk_instance, &devices, device_list), "physical device detection")
    vk_phys_device = device_list[0];

    //ToDo(facts) poll device properties properly
    // But I only have one GPU so its fine for now.


    for (i32 i = 0; i < devices; i++)
    {
        VkPhysicalDeviceProperties vk_phys_device_props = { 0 };
        vkGetPhysicalDeviceProperties(device_list[i], &vk_phys_device_props);

        if (vk_phys_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            device->vk_phys_device = device_list[i];
            log_device(&vk_phys_device_props)
            break;
        }
    }


    log_extention(check_device_extension_support(device->vk_phys_device))

    //set up queues

#define max_queues 5
    u32 queues = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->vk_phys_device, &queues, 0);

    Assert(queues <= max_queues, "More queues found than supported")

    VkQueueFamilyProperties vk_q_fam_prop_list[max_queues];
    u32 current_queues = max_queues;
    vkGetPhysicalDeviceQueueFamilyProperties(device->vk_phys_device, &current_queues, vk_q_fam_prop_list);

    i32 present_queue_found = -1;
    for (i32 i = 0; i < current_queues; i++)
    {
        VkQueueFlags qflags = vk_q_fam_prop_list[i].queueFlags;
        if (qflags & VK_QUEUE_GRAPHICS_BIT)
        {
            device->qfams[Q_FAM_GFX] = i;
        }

        if ((qflags & VK_QUEUE_GRAPHICS_BIT) && (qflags & VK_QUEUE_COMPUTE_BIT))
        {
            device->qfams[Q_FAM_GFX_COMPUTE] = i;
        }

        if (present_queue_found == -1)
        {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device->vk_phys_device, i, context->surface, &present_support);

            if (present_support == VK_TRUE)
            {
                device->qfams[Q_FAM_PRESENT] = i;
            }

            present_queue_found = 1;

        }

    }

    //Note(facts): They're all definitely the same family. Return to this when you understand the API better

    Assert(device->qfams[Q_FAM_GFX] != -1, "Graphics Queue not found")
    Assert(device->qfams[Q_FAM_GFX_COMPUTE] != -1, "Graphics Compute Queue not found")
    Assert(device->qfams[Q_FAM_PRESENT] != -1, "Present Queue not found")

    //create logical device


    //Logical Device starts here
    float queue_priority = 1;

    // Note(facts): The graphic , compute and present queue families are all index 0. And this is common behaviour. For now I am leaving this
    // like this. Later, I will make sure that incase they are different families, each gets its own queue

    VkDeviceQueueCreateInfo vk_device_q_create_info = { 0 };
    vk_device_q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_device_q_create_info.pNext = 0;
    vk_device_q_create_info.flags = 0;
    vk_device_q_create_info.queueFamilyIndex = Q_FAM_GFX;
    //this is number of queues you want to create. Not how many queues are available in that queue family
    vk_device_q_create_info.queueCount = 1;
    vk_device_q_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR vk_dynamic_rendering_feature = { 0 };
    vk_dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    vk_dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    const char* device_extention_names[2] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

    VkDeviceCreateInfo vk_device_create_info = { 0 };
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

    VkResultAssert(vkCreateDevice(device->vk_phys_device, &vk_device_create_info, 0, &device->vk_device), "Vulkan device creation");

    // ToDo(facts 12/24 0439): Its all one queue. Come back to this later. If even one gfx card is made where the queues are literally different,
    // I will account for them
    vkGetDeviceQueue(device->vk_device, Q_FAM_GFX, 0, &device->gfx_q);
}
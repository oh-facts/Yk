#include <renderer/context.h>


#if defined (_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif



enum Q_FAM
{
    Q_FAM_GFX,
    Q_FAM_GFX_COMPUTE,
    Q_FAM_PRESENT,
    Q_FAM_SIZE
};


void mn_context_innit(mn_context* context)
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


   
  
}

void mn_context_free(mn_context* context)
{
#if VK_USE_VALIDATION_LAYERS
    PFN_vkDestroyDebugUtilsMessengerEXT phunk = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->vk_instance, "vkDestroyDebugUtilsMessengerEXT");
    phunk(context->vk_instance, context->debug_messenger, 0);
#endif

    vkDestroyInstance(context->vk_instance, 0);
}
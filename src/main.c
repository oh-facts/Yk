#include <windows.h>
#include <yk.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#if defined (_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

//ToDo(facts): Better Debug profiles.
// 11/23 1758
//ToDo(facts): Fix flickering triangle (sync problem I think).
//ToDo(facts): Destroy resources
//ToDo(facts): Posix window so my linux friends can see my triangle
//ToDo(facts): Renderer Abstraction :skull:

#define DEBUG 1
#define VkDEBUG 0

#if DEBUG
    #define Assert(Expression, msg) if(!(Expression)) {printf("Fatal: %s",msg); *(int *)0 = 0;}
#else
    #define Assert(Expression)
#endif

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 0

void _print_device_details(VkPhysicalDeviceProperties* vk_phys_device_props)
{
    printf("\nSelected Gpu\n");
    printf("----------\n");
    printf("%s\n%u\n%s\n", vk_phys_device_props->deviceName, vk_phys_device_props->driverVersion, vk_phys_device_props->deviceType==2?"Discrete GPU" : "Integreted probably");
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
    VkExtensionProperties* availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    printf("Vulkan Device Available Extentions ");
    for (uint32_t j = 0; j < extensionCount; ++j)
    {
           
        printf("%s\n", availableExtensions[j]);
           
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

    if (result == VK_INCOMPLETE) {
        return;
    }
    else
    {
       *(int*)0 = 0;
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

#define Kilobytes(Value) ((uint64_t)(Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

struct YkMemory
{
    int is_initialized;
    u64 perm_storage_size;
    void *perm_storage;
    u64 temp_storage_size;
    void *temp_storage;
};

typedef struct YkMemory YkMemory;

#if DEBUG
    LPVOID base_address = (LPVOID)Terabytes(2);
#else
    LPVOID base_address = 0;
#endif

int main(int argc, char *argv[])
{

    HWND window_handle;

    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "MainWindowClass";

    if (!RegisterClass(&wc))
        exit(-1);

    window_handle = CreateWindowA(wc.lpszClassName, "yekate", WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, WIN_SIZE_X, WIN_SIZE_Y,
                                  NULL, NULL, wc.hInstance, NULL);

    if (!window_handle)
        exit(-1);

    ShowWindow(window_handle, SW_SHOWNORMAL);
    UpdateWindow(window_handle);

    YkMemory engine_memory = {0};
    engine_memory.perm_storage_size = Megabytes(64);
    engine_memory.temp_storage_size = Megabytes(64);

    u64 total_size = engine_memory.perm_storage_size + engine_memory.temp_storage_size;

    engine_memory.perm_storage = VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    engine_memory.temp_storage = (u8*)engine_memory.perm_storage + engine_memory.perm_storage_size;



    // vulkan stuff here

    //Note(facts): This will go to its own file later. Until I understand vulkan and win32 api enough, I will use main.c
    //This will continue until hello triangle

    log_extention(check_instance_extension_support())


    VkApplicationInfo vk_app_info = { 0 };
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pNext = 0;
    vk_app_info.pApplicationName = "yekate";
    vk_app_info.applicationVersion = 0;
    vk_app_info.pEngineName = "yk";
    vk_app_info.engineVersion = 0;
    vk_app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo vk_create_info = {0};
    vk_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_create_info.pNext = 0;
    vk_create_info.flags = 0;
    vk_create_info.pApplicationInfo = &vk_app_info;

#if VK_USE_VALIDATION_LAYERS
   
    const char* validation_layers[1] = { 0 };
    validation_layers[0] = "VK_LAYER_KHRONOS_validation";

    //validation layer support check
    

    vk_create_info.enabledLayerCount = 1;
    vk_create_info.ppEnabledLayerNames = validation_layers;

#endif

    /*
        When they adding constexpr to C fr fr
    */
    #define num_extensions 2
    const char* enabled_extensions[num_extensions] = {0};
    enabled_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    
    #if defined(_WIN32)
        enabled_extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    #elif defined(__ANDROID__)
        enabled_extensions[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
    #elif defined(__linux__)
        enabled_extensions[1] = (VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    #endif


    vk_create_info.enabledExtensionCount = num_extensions;
    vk_create_info.ppEnabledExtensionNames = enabled_extensions;

    VkInstance vk_instance;
    VkResultAssert(vkCreateInstance(&vk_create_info, 0, &vk_instance), "Vulkan instance creation")

   //Needs to be done first because queues need to be able to present and for that I need a surface
   //34.2.3
   //Win 32 surface platform

    VkWin32SurfaceCreateInfoKHR vk_win32_surface_create_info_khr = { 0 };
    vk_win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vk_win32_surface_create_info_khr.pNext = 0;
    vk_win32_surface_create_info_khr.flags = 0;
    vk_win32_surface_create_info_khr.hinstance = wc.hInstance;
    vk_win32_surface_create_info_khr.hwnd = window_handle;

    //ToDo(facts): Needs to be destroyed on exit. I lost track of what all needs to be destroyed on exit, but this is one of them
    VkSurfaceKHR vk_surface_khr = { 0 };
    VkResultAssert(vkCreateWin32SurfaceKHR(vk_instance, &vk_win32_surface_create_info_khr, 0, &vk_surface_khr), "Win 32 Surface Creation");


    // 5.1 starts here
    //Physical Device
    VkPhysicalDevice vk_phys_device = { 0 };

    {
        #define max_devices 3

        int devices = 0;
        vkEnumeratePhysicalDevices(vk_instance, &devices, 0);

        Assert(devices <= max_devices, "More than 3 graphics cards? Wth?")

        VkPhysicalDevice device_list[max_devices] = { 0 };

        VkResultAssert(vkEnumeratePhysicalDevices(vk_instance, &devices, device_list), "physical device detection")
        vk_phys_device = device_list[0];
      
        //ToDo(facts) poll device properties properly
        // But I only have one GPU so its fine for now.

   
        for (i32 i = 0; i < devices; i++)
        {
            VkPhysicalDeviceProperties vk_phys_device_props = { 0 };
            vkGetPhysicalDeviceProperties(device_list[i], &vk_phys_device_props);

            if (vk_phys_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                vk_phys_device = device_list[i];
                log_device(&vk_phys_device_props)
                break;
            }
        }
        
    }
    log_extention(check_device_extension_support(vk_phys_device))


    //5.2 starts here
    
    //Queues
    i32 vk_graphics_qfam = { -1 };
    i32 vk_graphics_compute_qfam = { -1 };
    i32 vk_present_qfam = { -1 };
    {
        //Nvidia 4090 has 5.
        #define max_queues 5
        int queues = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_phys_device, &queues, 0);

        Assert(queues <= max_queues, "More queues found than supported")

        VkQueueFamilyProperties vk_q_fam_prop_list[max_queues];
        int current_queues = max_queues;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_phys_device, &current_queues, vk_q_fam_prop_list);

       

        for (i32 i = 0; i < current_queues; i++)
        {
            VkQueueFlags qflags = vk_q_fam_prop_list[i].queueFlags;
            if (qflags & VK_QUEUE_GRAPHICS_BIT)
            {
                vk_graphics_qfam = i;
            }

            if ((qflags & VK_QUEUE_GRAPHICS_BIT) && (qflags & VK_QUEUE_COMPUTE_BIT))
            {
                vk_graphics_compute_qfam = i;
            }

            if (vk_present_qfam == -1)
            {
                VkBool32 present_support = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(vk_phys_device, i, vk_surface_khr, &present_support);

                if (present_support == VK_TRUE)
                {
                    vk_present_qfam = i;
                }
            }
           
        }

    }
    Assert(vk_graphics_qfam != -1, "Graphics Queue not found")
    Assert(vk_graphics_compute_qfam != -1, "Graphics Compute Queue not found")
    Assert(vk_present_qfam != -1, "Present Queue not found")


    //Logical Device starts here
    float queue_priority = 1;

    // Note(facts): The graphic , compute and present queue families are all index 0. And this is common behaviour. For now I am leaving this
    // like this. Later, I will make sure that incase they are different families, each gets its own queue

    VkDeviceQueueCreateInfo vk_device_q_create_info = { 0 };
    vk_device_q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_device_q_create_info.pNext = 0;
    vk_device_q_create_info.flags = 0;
    vk_device_q_create_info.queueFamilyIndex = vk_graphics_qfam;
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

    VkDevice vk_device;
    VkResultAssert(vkCreateDevice(vk_phys_device, &vk_device_create_info, 0, &vk_device), "Vulkan device creation");
    
    // same as present queue for now
    VkQueue vk_graphics_q = { 0 };
    vkGetDeviceQueue(vk_device, vk_graphics_qfam, 0, &vk_graphics_q);

   //more 32

    //https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html
    VkSurfaceCapabilitiesKHR vk_surface_caps = { 0 };
    VkResultAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_phys_device, vk_surface_khr, &vk_surface_caps), "Surface Capabilities poll");

    Assert(vk_surface_caps.maxImageCount >= 1, "Max images supported than 1");
    uint32_t imageCount = vk_surface_caps.minImageCount + 1;
    if (imageCount > vk_surface_caps.maxImageCount)
        imageCount = vk_surface_caps.maxImageCount;

    VkExtent2D vk_extent = { 0 };
    if (vk_surface_caps.currentExtent.width == -1 || vk_surface_caps.currentExtent.height == -1)
    {
        vk_extent.width = WIN_SIZE_X;
        vk_extent.height = WIN_SIZE_Y;
    }
    else
    {
        vk_extent = vk_surface_caps.currentExtent;
    }

    //ToDo(facts): Start doing this on the heap

    #define max_format_count 5
    u32 vk_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_phys_device, vk_surface_khr, &vk_format_count, 0);
    Assert(vk_format_count > 0, "Format count less than 1")
    Assert(vk_format_count <= max_format_count, "Too many formats")

    VkSurfaceFormatKHR vk_surface_format_list[max_format_count] = { 0 };
    VkResultAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_phys_device, vk_surface_khr, &vk_format_count, vk_surface_format_list), "Surface formats obtain")

    //ToDo(facts, 12/22): Stop being a smartass at 5:58am. Go to sleep
    VkSurfaceFormatKHR surface_format = { 0 };
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
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_phys_device, vk_surface_khr, &vk_present_mode_count, 0);
    Assert(vk_present_mode_count > 0, "Less than 1 present modes found")
    Assert(vk_present_mode_count <= max_present_mode, "Too many present modes")

    VkPresentModeKHR vk_present_mode_list[max_present_mode] = { 0 };

    VkResultAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(vk_phys_device, vk_surface_khr, &vk_present_mode_count, vk_present_mode_list), "Device Present Modes")

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
    
    u32 vk_qfam_indices[1] = { 0 };
    VkSwapchainCreateInfoKHR vk_swapchain_create_info = { 0 };
    vk_swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vk_swapchain_create_info.pNext = 0;
    vk_swapchain_create_info.flags = 0;
    vk_swapchain_create_info.surface = vk_surface_khr;
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



    VkSwapchainKHR vk_swapchain = { 0 };
    VkResultAssert(vkCreateSwapchainKHR(vk_device, &vk_swapchain_create_info, 0, &vk_swapchain), "Created Swapchain");


    //Images here
    // my gpu can support 8. I will go with 3. I think 2 is better.
    // Come back to this when you understand this better
    #define max_images 3

    i32 vk_image_num = 0;
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &vk_image_num, 0);
    Assert(vk_image_num <= max_images, "More swapchain images than expected")

    VkImage vk_swapchain_image_list[max_images] = { 0 };
    VkResultAssert(vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &vk_image_num, vk_swapchain_image_list), "Swapchain images found");

    VkImageView vk_swapchain_image_view_list[max_images] = { 0 };

    for (i32 i = 0; i < vk_image_num; i++)
    {
        VkImageViewCreateInfo vk_image_view_create_info = { 0 };
        vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vk_image_view_create_info.pNext = 0;
        vk_image_view_create_info.image = vk_swapchain_image_list[i];
        vk_image_view_create_info.viewType = VK_IMAGE_TYPE_2D;
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
        VkResultAssert(vkCreateImageView(vk_device, &vk_image_view_create_info, 0, &vk_swapchain_image_view_list[i]), str);
    }

    //Graphics Pipeline starts here

    size_t vert_len = 0;
    size_t frag_len = 0;
    const char* vert_shader_code = yk_read_binary_file("res/vert.spv", &vert_len);
    const char* frag_shader_code = yk_read_binary_file("res/frag.spv", &frag_len);

    VkShaderModuleCreateInfo vk_vert_shader_module_create_info = { 0 };
    vk_vert_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vk_vert_shader_module_create_info.codeSize = vert_len;
    vk_vert_shader_module_create_info.pCode = (u32*)vert_shader_code;

    VkShaderModule vk_vert_shader_module = { 0 };
    VkResultAssert(vkCreateShaderModule(vk_device, &vk_vert_shader_module_create_info, 0, &vk_vert_shader_module), "Vert Shader Module Creation");

    VkShaderModuleCreateInfo vk_frag_shader_module_create_info = { 0 };
    vk_frag_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vk_frag_shader_module_create_info.codeSize = frag_len;
    vk_frag_shader_module_create_info.pCode = (u32*)frag_shader_code;

    VkShaderModule vk_frag_shader_module = { 0 };
    VkResultAssert(vkCreateShaderModule(vk_device, &vk_frag_shader_module_create_info, 0, &vk_frag_shader_module), "Frag Shader Module Creation");

    free(vert_shader_code);
    free(frag_shader_code);

    VkPipelineShaderStageCreateInfo vk_vert_shader_stage_info = {0};
    vk_vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vk_vert_shader_stage_info.module = vk_vert_shader_module;
    vk_vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo vk_frag_shader_stage_info = { 0 };
    vk_frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vk_frag_shader_stage_info.module = vk_frag_shader_module;
    vk_frag_shader_stage_info.pName = "main";


    VkPipelineShaderStageCreateInfo vk_shader_stages[] = { vk_vert_shader_stage_info, vk_frag_shader_stage_info };

    //Note(facts 12/23 0223) : Complete pipeline. Then rendering.

    VkDynamicState vk_dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo vk_dyn_state_create_info = { 0 };
    vk_dyn_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    vk_dyn_state_create_info.dynamicStateCount = 2;
    vk_dyn_state_create_info.pDynamicStates = vk_dynamic_states;

    VkPipelineVertexInputStateCreateInfo vk_vertex_input_info = { 0 };
    vk_vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo vk_input_asm = { 0 };
    vk_input_asm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vk_input_asm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vk_input_asm.primitiveRestartEnable = VK_FALSE;

    VkViewport vk_viewport = { 0 };
    vk_viewport.x = 0.0f;
    vk_viewport.y = 0.0f;
    vk_viewport.width = (f32)vk_extent.width;
    vk_viewport.height = (f32)vk_extent.height;
    vk_viewport.minDepth = 0.0f;
    vk_viewport.maxDepth = 1.0f;

    VkRect2D vk_scissor = { 0 };
    vk_scissor.offset = (VkOffset2D){ 0,0 };
    vk_scissor.extent = vk_extent;

    VkPipelineViewportStateCreateInfo vk_viewport_state = { 0 };
    vk_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vk_viewport_state.viewportCount = 1;
    vk_viewport_state.pViewports = &vk_viewport;
    vk_viewport_state.scissorCount = 1;
    vk_viewport_state.pScissors = &vk_scissor;

    VkPipelineRasterizationStateCreateInfo vk_rasterizer = { 0 };
    vk_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vk_rasterizer.depthClampEnable = VK_FALSE;
    vk_rasterizer.rasterizerDiscardEnable = VK_FALSE;
    vk_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    vk_rasterizer.lineWidth = 1.0f;
    vk_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    vk_rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    //Note(facts 12/23 2:20): come back to this later
    vk_rasterizer.depthBiasEnable = VK_FALSE;
    vk_rasterizer.depthBiasConstantFactor = 0.0f;
    vk_rasterizer.depthBiasClamp = 0.0f; 
    vk_rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo vk_multisampling = { 0 };
    vk_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vk_multisampling.sampleShadingEnable = VK_FALSE;
    vk_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    vk_multisampling.minSampleShading = 1.0f;
    vk_multisampling.pSampleMask = 0;
    vk_multisampling.alphaToCoverageEnable = VK_FALSE;
    vk_multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState vk_color_blend_attatchment = { 0 };
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
    VkPipelineColorBlendStateCreateInfo vk_color_blending = { 0 };
    vk_color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vk_color_blending.logicOpEnable = VK_FALSE;
    vk_color_blending.logicOp = VK_LOGIC_OP_COPY;
    vk_color_blending.attachmentCount = 1;
    vk_color_blending.pAttachments = &vk_color_blend_attatchment;
    vk_color_blending.blendConstants[0] = 0.0f;
    vk_color_blending.blendConstants[1] = 0.0f;
    vk_color_blending.blendConstants[2] = 0.0f;
    vk_color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo vk_pipeline_layout_info = { 0 };
    vk_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_pipeline_layout_info.setLayoutCount = 0; 
    vk_pipeline_layout_info.pSetLayouts = 0; 
    vk_pipeline_layout_info.pushConstantRangeCount = 0;  
    vk_pipeline_layout_info.pPushConstantRanges = 0;

    VkPipelineLayout vk_pipeline_layout = { 0 };
    
    VkResultAssert(vkCreatePipelineLayout(vk_device, &vk_pipeline_layout_info, 0, &vk_pipeline_layout), "Pipeline layout creation");

    //graphics pipeline.
    //we want to do dynamic rendering
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info = { 0 };
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

    VkRenderingAttachmentInfoKHR vk_color_attachment = { 0 };
    vk_color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_color_attachment.imageView = vk_swapchain_image_view_list[0];
    vk_color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vk_color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vk_color_attachment.clearValue.color = (VkClearColorValue){ 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingAttachmentInfoKHR vk_depth_attachment = { 0 };
    vk_depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_depth_attachment.imageView = vk_swapchain_image_view_list[1];
    vk_depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    vk_depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vk_depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vk_depth_attachment.clearValue.depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };

    VkRenderingAttachmentInfoKHR vk_stencil_attachment = { 0 };
    vk_stencil_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    vk_stencil_attachment.imageView = vk_swapchain_image_view_list[2];
    vk_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    vk_stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vk_stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vk_stencil_attachment.clearValue.depthStencil = (VkClearDepthStencilValue){ 0.0f, 0 };


    VkRenderingInfoKHR vk_rendering_info = { 0 };
    vk_rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    vk_rendering_info.pNext = 0;
    vk_rendering_info.flags = 0;
    vk_rendering_info.renderArea = vk_scissor;
    vk_rendering_info.layerCount = 1;
    vk_rendering_info.viewMask = 0;
    vk_rendering_info.colorAttachmentCount = 1;
    vk_rendering_info.pColorAttachments = &vk_color_attachment;
    vk_rendering_info.pDepthAttachment = VK_NULL_HANDLE; //&vk_depth_attachment;
    vk_rendering_info.pStencilAttachment = VK_NULL_HANDLE; //&vk_stencil_attachment;

    VkPipeline vk_graphics_pipeline = { 0 };
    VkResultAssert(vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &vk_graphics_pipeline_create_info, 0, &vk_graphics_pipeline), "Graphics pipeline creation");


    //6 starts here
    // These happen after swapchain and image so I moved it down
    // Commands are issued to do things. Commands are created from command pools
    VkCommandPoolCreateInfo vk_cmd_pool_create_info = { 0 };
    vk_cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vk_cmd_pool_create_info.pNext = 0;
    vk_cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_cmd_pool_create_info.queueFamilyIndex = vk_graphics_qfam;

    VkCommandPool vk_cmd_pool;
    VkResultAssert(vkCreateCommandPool(vk_device, &vk_cmd_pool_create_info, 0, &vk_cmd_pool), "Command pool creation");

    VkCommandBufferAllocateInfo vk_cmd_buffer_alloc_info = { 0 };
    vk_cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vk_cmd_buffer_alloc_info.pNext = 0;
    vk_cmd_buffer_alloc_info.commandPool = vk_cmd_pool;
    vk_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk_cmd_buffer_alloc_info.commandBufferCount = 1;

    VkCommandBuffer vk_cmd_buffer;
    VkResultAssert(vkAllocateCommandBuffers(vk_device, &vk_cmd_buffer_alloc_info, &vk_cmd_buffer), "Command Buffer allocation");

   


    //https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
    
    //Some 7 stuff. I need semawhores
    
    //if semaphores aren't extended with semaphore types, they will be binary
    
    VkSemaphore vk_image_available_semawhore = { 0 };
    VkSemaphore vk_render_finished_semawhore = { 0 };
    VkFence vk_in_flight_fence = { 0 };

    VkSemaphoreCreateInfo vk_semawhore_create_info = { 0 };
    vk_semawhore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk_semawhore_create_info.pNext = 0;
    vk_semawhore_create_info.flags = 0;
    
    VkFenceCreateInfo vk_fence_create_info = { 0 };
    vk_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    VkResultAssert(vkCreateSemaphore(vk_device, &vk_semawhore_create_info, 0, &vk_image_available_semawhore), "Image ready semaphore");
    VkResultAssert(vkCreateSemaphore(vk_device, &vk_semawhore_create_info, 0, &vk_render_finished_semawhore), "Render finished semaphore");
    VkResultAssert(vkCreateFence(vk_device, &vk_fence_create_info, 0, &vk_in_flight_fence), "flight fence");

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(vk_device, "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(vk_device, "vkCmdEndRenderingKHR");

    

    
    
  
    while (1)
    {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        vkWaitForFences(vk_device, 1, &vk_in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(vk_device, 1, &vk_in_flight_fence);

        uint32_t imageIndex = -1;
        vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_image_available_semawhore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(vk_cmd_buffer, 0);

        //command buffer record

        VkCommandBufferBeginInfo vk_cmd_buffer_begin_info = { 0 };
        vk_cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk_cmd_buffer_begin_info.pNext = 0;
        //0 is fine. flags are mean for specific cases
        vk_cmd_buffer_begin_info.flags = 0;
        //for secondary buffers
        vk_cmd_buffer_begin_info.pInheritanceInfo = 0;

        VkResultAssert(vkBeginCommandBuffer(vk_cmd_buffer, &vk_cmd_buffer_begin_info), "Command buffer begin");

        // Begin rendering
        vkCmdBeginRenderingKHR(vk_cmd_buffer, &vk_rendering_info);

        // Bind the graphics pipeline
        vkCmdBindPipeline(vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_graphics_pipeline);

        // Set the viewport and scissor
        vkCmdSetViewport(vk_cmd_buffer, 0, 1, &vk_viewport);
        vkCmdSetScissor(vk_cmd_buffer, 0, 1, &vk_scissor);

        // Issue the draw command to draw the triangle
        vkCmdDraw(vk_cmd_buffer, 3, 1, 0, 0);

        // End rendering
        vkCmdEndRenderingKHR(vk_cmd_buffer);

        VkImageMemoryBarrier barrier = { 0 };
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        Assert(imageIndex < max_images, "Too many images.")

        barrier.image = vk_swapchain_image_list[imageIndex];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        vkCmdPipelineBarrier(
            vk_cmd_buffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, 0,
            0, 0,
            1, &barrier
        );

        VkResultAssert(vkEndCommandBuffer(vk_cmd_buffer), "Command buffer end");

        //command buffer recording over

        VkSubmitInfo vk_submit_info = { 0 };
        vk_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vk_submit_info.pNext = 0;

        VkSemaphore vk_wait_semawhores[] = { vk_image_available_semawhore };
        VkPipelineStageFlags vk_wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        vk_submit_info.waitSemaphoreCount = 1;
        vk_submit_info.pWaitSemaphores = vk_wait_semawhores;
        vk_submit_info.pWaitDstStageMask = vk_wait_stages;

        vk_submit_info.commandBufferCount = 1;
        vk_submit_info.pCommandBuffers = &vk_cmd_buffer;

        VkSemaphore vk_signal_semawhores[] = { vk_render_finished_semawhore };
        vk_submit_info.signalSemaphoreCount = 1;
        vk_submit_info.pSignalSemaphores = vk_signal_semawhores;

        VkResultAssert(vkQueueSubmit(vk_graphics_q, 1, &vk_submit_info, vk_in_flight_fence), "Draw command buffer submitted");

        VkPresentInfoKHR vk_present_info = { 0 };
        vk_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        vk_present_info.waitSemaphoreCount = 1;
        vk_present_info.pWaitSemaphores = vk_signal_semawhores;

        VkSwapchainKHR vk_swapchains[] = { vk_swapchain };
        vk_present_info.swapchainCount = 1;
        vk_present_info.pSwapchains = vk_swapchains;
        vk_present_info.pImageIndices = &imageIndex;

        vk_present_info.pResults = 0;

        //present q same as graphics for now
        VkResultAssert(vkQueuePresentKHR(vk_graphics_q, &vk_present_info), "Present queue")
            printf("we");
    }
    vkDeviceWaitIdle(vk_device);
    // ToDo(facts 11/22 16:22): Remember to destroy window
    //vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, 0);
    /*
    for (i32 i = 0; i < vk_image_num; i++)
    {
        vkDestroyImageView(vk_device, vk_swapchain_image_view_list[i], 0);
    }
   
    */

    //vkDestroyShaderModule(vk_device, vk_shader_frag_module, 0);
    //vkDestroyShaderModule(vk_device, vk_shader_vert_module, 0);
    // vkDestroySwapchainKHR(vk_device, vk_swapchain, 0);
    // vkDestroyDevice(vk_device, 0);
    // vkDestroySurfaceKHR(vk_instance, vk_surface, 0);
    // vkDestroyInstance(vk_instance, 0);

    return 0;
}
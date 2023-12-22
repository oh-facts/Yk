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

#define DEBUG 1

#if DEBUG
    #define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
    #define Assert(Expression)
#endif

#define VK_EXT_PRINT_DEBUG 0

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
        printf("%s\n", extensions[i].extensionName);
    }
    printf("\n");

    free(extensions);
}

void _check_vk_result(VkResult result, const char* msg) {

    if (result == VK_SUCCESS)
    {
        return;
    }
    
    const char* error_msg = string_VkResult(result);
    printf("%s: %s\n", msg, error_msg);

    if (result == VK_INCOMPLETE) {
        return;
    }
    else
    {
       *(int*)0 = 0;
    }
 
}

#if DEBUG
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
                                  CW_USEDEFAULT, CW_USEDEFAULT, 600, 600,
                                  NULL, NULL, wc.hInstance, NULL);

    if (!window_handle)
        exit(-1);

    ShowWindow(window_handle, SW_SHOWNORMAL);
    UpdateWindow(window_handle);

    YkMemory engine_memory = {0};
    engine_memory.perm_storage_size = Megabytes(64);
    engine_memory.temp_storage_size = Gigabytes(4);

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

    const char* validation_layers[1] = { 0 };
    validation_layers[0] = "VK_LAYER_KHRONOS_validation";

    //validation layer support check
    

    vk_create_info.enabledLayerCount = 1;
    vk_create_info.ppEnabledLayerNames = validation_layers;

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
    VkResultAssert(vkCreateInstance(&vk_create_info, 0, &vk_instance) , "VkInstance creation failed.")
    

// 5.1 starts here
    #define max_devices 3
  
    int devices = 0;
    vkEnumeratePhysicalDevices(vk_instance, &devices, 0);
    
    Assert(devices <= max_devices)

    VkPhysicalDevice device_list[max_devices] = { 0 };
    int devices_used = max_devices;
    
    VkResultAssert(vkEnumeratePhysicalDevices(vk_instance, &devices_used, device_list), "enumerate physical devices failed")

    log_extention(check_device_extension_support(device_list[0]))

    //ToDo(facts) poll device properties properly
    // But I only have one GPU so its fine for now.
    VkPhysicalDeviceProperties yk_device_props;
    vkGetPhysicalDeviceProperties(device_list[0], &yk_device_props);
    
//5.2 starts here
    //Nvidia 4090 has 5.
    #define max_queues 5
    int queues = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device_list[0], &queues, 0);
    
    Assert(queues <= max_queues)

    VkQueueFamilyProperties vk_q_fam_prop_list[max_queues];
    int queues_used = max_queues;

    vkGetPhysicalDeviceQueueFamilyProperties(device_list[0], &queues_used, vk_q_fam_prop_list);

    float queue_priority = 1;

    VkDeviceQueueCreateInfo vk_device_q_create_info = { 0 };
    vk_device_q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_device_q_create_info.pNext = 0;
    vk_device_q_create_info.queueFamilyIndex = 0;
    //this is number of queues you want to create. Not how many queues are available in that queue family
    vk_device_q_create_info.queueCount = 1;
    vk_device_q_create_info.pQueuePriorities = &queue_priority;
    
    const char* device_extention_names[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo vk_device_create_info = { 0 };   
    vk_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vk_device_create_info.pNext = 0;
    vk_device_create_info.flags = 0;
    vk_device_create_info.queueCreateInfoCount = 1;
    vk_device_create_info.pQueueCreateInfos = &vk_device_q_create_info;
    vk_device_create_info.enabledLayerCount = 0;
    vk_device_create_info.ppEnabledLayerNames = 0;
    vk_device_create_info.enabledExtensionCount = 1;
    vk_device_create_info.ppEnabledExtensionNames = device_extention_names;
    vk_device_create_info.pEnabledFeatures = 0;


    
    

    VkDevice vk_device;
    VkResultAssert(vkCreateDevice(device_list[0], &vk_device_create_info, 0, &vk_device), "Vulkan device creation failed");
    
    MSG msg;
    while (1)
    {
        BOOL message_result = GetMessage(&msg, 0, 0, 0);
        if (message_result > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            break;
        }
    }

    return 0;
}
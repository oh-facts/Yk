#include <windows.h>
#include <yk.h>
#include <vulkan/vulkan.h>

#if defined (_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

#define DEBUG 1

#if DEBUG
    #define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
    #define Assert(Expression)
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

    char* validation_layers[1];
    validation_layers[0] = "VK_LAYER_KHRONOS_validation";

    vk_create_info.enabledLayerCount = 1;
    vk_create_info.ppEnabledLayerNames = validation_layers;

    /*
        When they adding constexpr to C fr fr
    */
    #define num_extensions 2
    char* enabled_extensions[num_extensions];
    enabled_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    #if defined(_WIN32)
    enabled_extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

    #elif defined(__ANDROID__)
        enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    #elif defined(__linux__)
        enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    #endif

    vk_create_info.enabledExtensionCount = num_extensions;
    vk_create_info.ppEnabledExtensionNames = enabled_extensions;

    VkInstance vk_instance;
    vkCreateInstance(&vk_create_info, 0, &vk_instance);
    
    uint32_t extensionCount = 17;
    VkExtensionProperties ext_prop[17];
    VkResult aw =  vkEnumerateInstanceExtensionProperties(0, &extensionCount, &ext_prop);
    

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
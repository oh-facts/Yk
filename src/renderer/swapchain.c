#include <renderer/swapchain.h>


#if defined (_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif



void mn_swapchain_innit(mn_context* context, mn_device* device, mn_swapchain_info swapchain_info, mn_swapchain* swapchain)
{
    //https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html

    VkSurfaceCapabilitiesKHR vk_surface_caps = { 0 };
    VkResultAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->phys_device, swapchain_info.surface, &vk_surface_caps), "Surface Capabilities poll");

    Assert(vk_surface_caps.maxImageCount >= 1, "Max images supported than 1");
    uint32_t imageCount = vk_surface_caps.minImageCount + 1;
    if (imageCount > vk_surface_caps.maxImageCount)
        imageCount = vk_surface_caps.maxImageCount;

    VkExtent2D vk_extent = { 0 };
    if (vk_surface_caps.currentExtent.width == -1 || vk_surface_caps.currentExtent.height == -1)
    {
        vk_extent.width = swapchain_info.win_size_x;
        vk_extent.height = swapchain_info.win_size_y;
    }
    else
    {
        vk_extent = vk_surface_caps.currentExtent;
    }

    //cursed fuckery
    swapchain->extent = vk_extent;

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

    swapchain->scissor = vk_scissor;
    swapchain->viewport = vk_viewport;


    //ToDo(facts): Start doing this on the heap

#define max_format_count 5
    u32 vk_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->phys_device, swapchain_info.surface, &vk_format_count, 0);
    Assert(vk_format_count > 0, "Format count less than 1")
    Assert(vk_format_count <= max_format_count, "Too many formats")

    VkSurfaceFormatKHR vk_surface_format_list[max_format_count] = { 0 };
    VkResultAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(device->phys_device, swapchain_info.surface, &vk_format_count, vk_surface_format_list), "Surface formats obtain")

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
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->phys_device, swapchain_info.surface, &vk_present_mode_count, 0);
    Assert(vk_present_mode_count > 0, "Less than 1 present modes found")
    Assert(vk_present_mode_count <= max_present_mode, "Too many present modes")

    VkPresentModeKHR vk_present_mode_list[max_present_mode] = { 0 };

    VkResultAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(device->phys_device, swapchain_info.surface, &vk_present_mode_count, vk_present_mode_list), "Device Present Modes")

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
    vk_swapchain_create_info.surface = swapchain_info.surface;
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

    VkResultAssert(vkCreateSwapchainKHR(device->handle, &vk_swapchain_create_info, 0, &swapchain->swapchain), "Created Swapchain");

#define max_images 3

    u32 vk_image_num = 0;
    vkGetSwapchainImagesKHR(device->handle, swapchain->swapchain, &vk_image_num, 0);
    Assert(vk_image_num <= max_images, "More swapchain images than expected")

    VkResultAssert(vkGetSwapchainImagesKHR(device->handle, swapchain->swapchain, &vk_image_num, swapchain->images), "Swapchain images found");




    for (i32 i = 0; i < vk_image_num; i++)
    {
        VkImageViewCreateInfo vk_image_view_create_info = { 0 };
        vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vk_image_view_create_info.pNext = 0;
        vk_image_view_create_info.image = swapchain->images[i];
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
        VkResultAssert(vkCreateImageView(device->handle, &vk_image_view_create_info, 0, &swapchain->image_views[i]), str);
    }

}

void mn_surface_innit(mn_context* context, void* window_handle, VkSurfaceKHR *surface)
{
    VkWin32SurfaceCreateInfoKHR vk_win32_surface_create_info_khr = { 0 };
    vk_win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vk_win32_surface_create_info_khr.pNext = 0;
    vk_win32_surface_create_info_khr.flags = 0;

#if defined(_WIN32)
    HWND win32_handle = (HWND)window_handle;
    vk_win32_surface_create_info_khr.hinstance = GetModuleHandleA(0);
    vk_win32_surface_create_info_khr.hwnd = win32_handle;
    VkResultAssert(vkCreateWin32SurfaceKHR(context->vk_instance, &vk_win32_surface_create_info_khr, 0, surface), "Win 32 Surface Creation");
#endif // DEBUG


}
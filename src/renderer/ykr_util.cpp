#include <renderer/ykr_util.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

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

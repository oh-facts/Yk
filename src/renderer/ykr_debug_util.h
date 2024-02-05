#ifndef YKR_DEBUG_UTIL_H
#define YKR_DEBUG_UTIL_H

/*
    Internal Util functions. No one outside this folder should see this
*/

#include <renderer/ykr_common.h>
void _print_device_details(VkPhysicalDeviceProperties* vk_phys_device_props);
    
#if LOG_DEVICE_DETAILS
#define log_device(Expression) _print_device_details(Expression);
#else
#define log_device(Expession)
#endif

void check_device_extension_support(VkPhysicalDevice device);

void check_instance_extension_support();

void _check_vk_result(VkResult result, const char* msg);


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

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);



#endif
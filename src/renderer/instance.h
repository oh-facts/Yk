#ifndef YK_RENDERER_INSTANCE_H
#define YK_RENDERER_INSTANCE_H

#include <renderer/renderer.h>

#define VkDEBUG 1

#define VK_USE_VALIDATION_LAYERS 1
#define VK_EXT_PRINT_DEBUG 0
#define VK_PRINT_SUCCESS 0
#define LOG_DEVICE_DETAILS 0
/*
	Vulkan instance and physical device
	The renderer is called moon internally.
	so mn prefix. This is merely to prevent me
	from writing YkRendererInstance. Since 
	YkRenderer is basically Moon. I can just do MnInstance.
*/

struct MnInstance
{
	VkInstance vk_instance;

#if VK_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

typedef struct MnInstance MnInstance;


void mn_instance_innit(MnInstance* self);
void mn_instance_free(MnInstance* self);


#endif
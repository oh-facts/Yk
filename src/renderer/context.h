#ifndef YK_RENDERER_CONTEXT_H
#define YK_RENDERER_CONTEXT_H

#include <yk.h>
#include <renderer/common.h>

typedef struct mn_context mn_context;

struct mn_context
{
	VkInstance vk_instance;
#if VK_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};


void mn_context_innit(mn_context* context);
void mn_context_free(mn_context* context);


#endif
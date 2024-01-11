#ifndef YKR_INSTANCE_H
#define YKR_INSTANCE_H
#include <renderer/ykr_common.h>
#include <renderer/yk_renderer.h>
//vk boiler plate

/*
    Pure vulkan related boilerplate
*/
void yk_innit_vulkan(YkRenderer* renderer);
void yk_create_surface(YkRenderer* renderer, void* native_handle);
void yk_pick_physdevice(YkRenderer* renderer);
void yk_find_queues(YkRenderer* renderer);
void yk_create_device(YkRenderer* renderer);
void yk_create_swapchain(YkRenderer* renderer, YkWindow* win);

#endif
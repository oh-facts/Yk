#ifndef RENDERER_H
#define RENDERER_H

#include <renderer/ykr_common.h>


#include <yk_api.h>

#include <platform/yk_window.h>
#include <renderer/yk_debug_camera.h>
#include <renderer/descriptors.h>
#include <pch.h>


struct ComputePushConstants {
	v4 data1;
	v4 data2;
	v4 data3;
	v4 data4;
};

/*
* Most of this is internal state and unrequired by other structs. Still, I don't know enough about vulkan to want to abstract this away.
* So this will be a megastruct until then.
*/

//https://github.com/KhronosGroup/Vulkan-Samples/blob/main/samples/api/hpp_hello_triangle/hpp_hello_triangle.h
struct yk_frame_data
{
	VkFence in_flight_fence;

	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffers;

	VkSemaphore image_available_semawhore;
	VkSemaphore render_finished_semawhore;
	
	YkBuffer scene_ubo;
	VkDescriptorSet scene_set;
};

struct YkRenderer
{
	VkInstance vk_instance;
	//ToDo(facts 12/24 0341): Does renderer receive a window handle? Or does the renderer own the window? I want to be able to support multiple windows
	//for whatever reason. I will get back to this later. For now, a window handle should be fine.
	VkSurfaceKHR surface;
	VkPhysicalDevice phys_device;
	VkDevice device;
	VkQueue gfx_q;
	i32 qfams[3];
	VkSwapchainKHR swapchain;

	VkViewport viewport;
	VkRect2D scissor;
	
	VkExtent2D sc_extent;
	VkImage sc_images[max_images];
	VkImageView sc_image_views[max_images];
	
	VkDescriptorPool global_pool;
	
	yk_frame_data frame_data[MAX_FRAMES_IN_FLIGHT];

	clock_t clock;

	uint32_t current_frame;

	AllocatedImage draw_image;
	AllocatedImage depth_image;

	VkDescriptorSet draw_image_desc;
	VkDescriptorSetLayout draw_image_layouts;

	VkPipeline gradient_pp;
	VkPipelineLayout gradient_pp_layouts;

	VkDescriptorPool scene_desc_pool;
	VkDescriptorSetLayout scene_desc_layout;

	VkPipeline pipelines[pipeline_type_num];
	VkPipelineLayout layouts[pipeline_type_num];

	texture_asset* textures;
	size_t texture_count;

	mesh_asset* test_meshes;
	size_t test_mesh_count;
	VkDescriptorPool mesh_desc_pool;
	VkDescriptorSetLayout mesh_desc_layout;

	VmaAllocator vma_allocator;

	u32 frames_rendered;

	VkFence imm_fence;
	VkCommandPool imm_cmdpool;
	VkCommandBuffer imm_cmd;


	YkDebugCamera cam;



#if VK_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};



enum Q_FAM
{
	Q_FAM_GFX,
	Q_FAM_GFX_COMPUTE,
	Q_FAM_PRESENT
};

void yk_free_renderer(YkRenderer* renderer);

void yk_renderer_wait(YkRenderer* renderer);

void yk_renderer_innit(YkRenderer* renderer, struct YkWindow* window);

void yk_renderer_innit_scene(YkRenderer* renderer);

void yk_renderer_draw(YkRenderer* renderer, YkWindow* win, f64 dt);

b8 yk_recreate_swapchain(YkRenderer* renderer, struct YkWindow* win);

YkMeshBuffer ykr_upload_mesh(const YkRenderer* renderer, YkVertex vertices[], u32 num_vertices, u32 indices[], u32 num_indices);

AllocatedImage ykr_create_image_from_data(const YkRenderer* renderer, void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);

#endif
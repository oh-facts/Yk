#ifndef  RENDERER_PIPELINE_H
#define	 RENDERER_PIPELINE_H

#include "device.h"
#include "swapchain.h"

typedef struct mn_pipeline mn_pipeline;

struct mn_pipeline
{
	VkPipelineLayout pipeline_layout;

	VkPipeline handle;
};

void mn_raster_pipeline_innit(mn_device* device,  mn_swapchain* swapchain, mn_pipeline* pipeline, 
	VkDescriptorSetLayout* desc_layout, VkVertexInputBindingDescription* binding_desc, VkVertexInputAttributeDescription* attrib_desc);

#endif // ! RENDERER_PIPELINE_H
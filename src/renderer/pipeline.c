#include <renderer/pipeline.h>
#include <shader.h>

//ToDo(facts): pipeline should receive shader info

void mn_raster_pipeline_innit(mn_device* device, mn_swapchain* swapchain, mn_pipeline* pipeline, 
    VkDescriptorSetLayout * desc_layout, VkVertexInputBindingDescription* binding_desc, VkVertexInputAttributeDescription* attrib_desc)
{
    VkShaderModule vk_vert_shader_module = { 0 };
    VkShaderModule vk_frag_shader_module = { 0 };

    mn_create_shader("res/vert.spv", device->handle, &vk_vert_shader_module);
    mn_create_shader("res/frag.spv", device->handle, &vk_frag_shader_module);

    VkPipelineShaderStageCreateInfo vk_vert_shader_stage_info = { 0 };
    vk_vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vk_vert_shader_stage_info.module = vk_vert_shader_module;
    vk_vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo vk_frag_shader_stage_info = { 0 };
    vk_frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vk_frag_shader_stage_info.module = vk_frag_shader_module;
    vk_frag_shader_stage_info.pName = "main";


    VkPipelineShaderStageCreateInfo vk_shader_stages[] = { vk_vert_shader_stage_info, vk_frag_shader_stage_info };

    //Note(facts 12/23 0223) : Complete pipeline. Then rendering.

    VkDynamicState vk_dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo vk_dyn_state_create_info = { 0 };
    vk_dyn_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    vk_dyn_state_create_info.dynamicStateCount = 2;
    vk_dyn_state_create_info.pDynamicStates = vk_dynamic_states;


    VkPipelineVertexInputStateCreateInfo vk_vertex_input_info = { 0 };
    vk_vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vk_vertex_input_info.vertexBindingDescriptionCount = 1;
    vk_vertex_input_info.pVertexBindingDescriptions = binding_desc;
    vk_vertex_input_info.vertexAttributeDescriptionCount = 2;
    vk_vertex_input_info.pVertexAttributeDescriptions = attrib_desc;

    VkPipelineInputAssemblyStateCreateInfo vk_input_asm = { 0 };
    vk_input_asm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vk_input_asm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vk_input_asm.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo vk_viewport_state = { 0 };
    vk_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vk_viewport_state.viewportCount = 1;
    vk_viewport_state.pViewports = &swapchain->viewport;
    vk_viewport_state.scissorCount = 1;
    vk_viewport_state.pScissors = &swapchain->scissor;

    VkPipelineRasterizationStateCreateInfo vk_rasterizer = { 0 };
    vk_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vk_rasterizer.depthClampEnable = VK_FALSE;
    vk_rasterizer.rasterizerDiscardEnable = VK_FALSE;
    vk_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    vk_rasterizer.lineWidth = 1.0f;
    vk_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    vk_rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    //Note(facts 12/23 2:20): come back to this later
    vk_rasterizer.depthBiasEnable = VK_FALSE;
    vk_rasterizer.depthBiasConstantFactor = 0.0f;
    vk_rasterizer.depthBiasClamp = 0.0f;
    vk_rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo vk_multisampling = { 0 };
    vk_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vk_multisampling.sampleShadingEnable = VK_FALSE;
    vk_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    vk_multisampling.minSampleShading = 1.0f;
    vk_multisampling.pSampleMask = 0;
    vk_multisampling.alphaToCoverageEnable = VK_FALSE;
    vk_multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState vk_color_blend_attatchment = { 0 };
    vk_color_blend_attatchment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk_color_blend_attatchment.blendEnable = VK_FALSE;
    vk_color_blend_attatchment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    vk_color_blend_attatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    vk_color_blend_attatchment.colorBlendOp = VK_BLEND_OP_ADD;
    vk_color_blend_attatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    vk_color_blend_attatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    vk_color_blend_attatchment.alphaBlendOp = VK_BLEND_OP_ADD;
    vk_color_blend_attatchment.blendEnable = VK_TRUE;
    vk_color_blend_attatchment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    vk_color_blend_attatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    vk_color_blend_attatchment.colorBlendOp = VK_BLEND_OP_ADD;
    vk_color_blend_attatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    vk_color_blend_attatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    vk_color_blend_attatchment.alphaBlendOp = VK_BLEND_OP_ADD;

    //ToDo(facts): I have no fucking idea what this means
    //I just want a triangle
    VkPipelineColorBlendStateCreateInfo vk_color_blending = { 0 };
    vk_color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vk_color_blending.logicOpEnable = VK_FALSE;
    vk_color_blending.logicOp = VK_LOGIC_OP_COPY;
    vk_color_blending.attachmentCount = 1;
    vk_color_blending.pAttachments = &vk_color_blend_attatchment;
    vk_color_blending.blendConstants[0] = 0.0f;
    vk_color_blending.blendConstants[1] = 0.0f;
    vk_color_blending.blendConstants[2] = 0.0f;
    vk_color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo vk_pipeline_layout_info = { 0 };
    vk_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_pipeline_layout_info.setLayoutCount = 1;
    vk_pipeline_layout_info.pSetLayouts = desc_layout;
    vk_pipeline_layout_info.pushConstantRangeCount = 0;
    vk_pipeline_layout_info.pPushConstantRanges = 0;

    VkPipelineLayout vk_pipeline_layout = { 0 };

    VkResultAssert(vkCreatePipelineLayout(device->handle, &vk_pipeline_layout_info, 0, &vk_pipeline_layout), "Pipeline layout creation");

    //graphics pipeline.
    //we want to do dynamic rendering
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info = { 0 };
    vk_graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vk_graphics_pipeline_create_info.stageCount = 2;
    vk_graphics_pipeline_create_info.pStages = vk_shader_stages;
    vk_graphics_pipeline_create_info.pDynamicState = &vk_dyn_state_create_info;
    vk_graphics_pipeline_create_info.pVertexInputState = &vk_vertex_input_info;
    vk_graphics_pipeline_create_info.pInputAssemblyState = &vk_input_asm;
    vk_graphics_pipeline_create_info.pViewportState = &vk_viewport_state;
    vk_graphics_pipeline_create_info.pRasterizationState = &vk_rasterizer;
    vk_graphics_pipeline_create_info.pMultisampleState = &vk_multisampling;
    vk_graphics_pipeline_create_info.pColorBlendState = &vk_color_blending;
    vk_graphics_pipeline_create_info.layout = vk_pipeline_layout;
    vk_graphics_pipeline_create_info.renderPass = VK_NULL_HANDLE;


    VkResultAssert(vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, 1, &vk_graphics_pipeline_create_info, 0, &pipeline->handle), "Graphics pipeline creation");

    vkDestroyShaderModule(device->handle, vk_frag_shader_module, 0);
    vkDestroyShaderModule(device->handle, vk_vert_shader_module, 0);

    //Tee hee
    pipeline->pipeline_layout = vk_pipeline_layout;

}

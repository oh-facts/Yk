#include "shader.h"
#include "device.h"
void mn_create_shader(const char* path, VkDevice device, VkShaderModule* shader)
{
	size_t len = 0;
	const char* shader_code = yk_read_binary_file(path, &len);

	VkShaderModuleCreateInfo shader_module_create_info = { 0 };
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = len;
	shader_module_create_info.pCode = (u32*)shader_code;
	
	VkResultAssert(vkCreateShaderModule(device, &shader_module_create_info, 0, shader), "Shader Module Creation");

	free((char*)shader_code);
}
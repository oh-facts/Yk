#ifndef YK_RENDERER_SHADER_H
#define YK_RENDERER_SHADER_H
#include <renderer/common.h>
#include <yk.h>
void mn_create_shader(const char* path, VkDevice device, VkShaderModule* shader);

#endif // !YK_RENDERER_SHADER
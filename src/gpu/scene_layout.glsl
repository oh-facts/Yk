layout (set = 0, binding = 0) uniform scene_data
{
	vec4 lightColor;
	vec4 lightPos;
};

layout (set = 1, binding = 0) uniform object_data
{
	mat4 model_mat;
};

layout(set = 1, binding = 1) uniform sampler2D displayTexture;

layout (set = 1, binding = 2) uniform material
{
	vec4 color;
};
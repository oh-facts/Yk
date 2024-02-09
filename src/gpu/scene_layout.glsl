layout (set = 0, binding = 0) uniform scene_data
{
	vec4 lightColor;
	vec4 lightPos;
};

layout (set = 1, binding = 0) uniform object_data
{
	mat4 model_mat;
};
#version 450

layout (set = 0, binding = 0) uniform scene_data
{
	vec4 ambient_color;
};

layout (location = 0) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = vec4(inColor * ambient_color.xyz, 1);
}
#version 450

#extension GL_GOOGLE_include_directive : require

#include "scene_layout.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 fragPos;
layout (location = 4) in mat4 a;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(ambient_pos.xyz - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 ambient = ambient_color.xyz * ambient_color.w;
	vec3 diffuse = diff * ambient_color.xyz;
	vec3 result = (ambient + diffuse) * inColor;
	outFragColor = vec4(result , 1);
}
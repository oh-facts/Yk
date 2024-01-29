#version 450

#extension GL_GOOGLE_include_directive : require

#include "scene_layout.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = vec4(inColor * ambient_color.xyz, 1);
}
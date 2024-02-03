#version 450

#extension GL_GOOGLE_include_directive : require

#include "scene_layout.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 fragPos;
layout (location = 4) in mat4 a;

layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 1) uniform sampler2D displayTexture;

void main() 
{
	//ambient_color.w * 
	vec3 ambient = 0.9 * ambient_color.xyz;

	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(ambient_pos.xyz - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * ambient_color.xyz;

	float specular_strength = 1.0;
	vec3 reflect_dir = reflect(-lightDir, norm);
	float spec = pow(max(dot(lightDir, reflect_dir), 0.0), 32);
	vec3 specular = specular_strength * spec * ambient_color.xyz;

	vec3 textureColor = texture(displayTexture, inUV).xyz;


	vec3 result;
	if(textureColor.x == 0)
	{
		result = (ambient + diffuse + specular) * inColor;
	}
	else
	{
		result = (ambient + diffuse + specular) * textureColor;
	}
	outFragColor = vec4(result, 1.0);

}
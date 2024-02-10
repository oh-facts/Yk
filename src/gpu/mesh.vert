#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "scene_layout.glsl"

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec3 fragPos;
layout (location = 4) out vec4 outMat;

struct Vertex {

	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 render_matrix;
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = PushConstants.render_matrix *vec4(v.position, 1.0f);
	outColor =  v.color;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	normal = mat3(transpose(inverse(model_mat))) * v.normal; 
	outMat = color;

	fragPos = vec3((model_mat * vec4(v.position, 1.0f)));

}
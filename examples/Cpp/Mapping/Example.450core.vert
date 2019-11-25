// GLSL texturing vertex shader
#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 vTexCoord;

void main()
{
	// Pass vertex data to tessellation control shader
	gl_Position = vec4(position, 0, 1);
	vTexCoord = texCoord;
}

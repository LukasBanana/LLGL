// GLSL model fragment shader

#version 450 core

layout(std140, binding = 0) uniform Settings
{
	mat4 wvpMatrix;
	vec4 color;
};

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = color;
}

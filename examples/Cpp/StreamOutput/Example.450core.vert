// GLSL vertex shader
#version 450 core

layout(binding = 2, std140) uniform Settings
{
	mat4 wvpMatrix;
};

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
}

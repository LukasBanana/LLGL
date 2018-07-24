// GLSL model vertex shader

#version 450 core

layout(std140, binding = 0) uniform Settings
{
	mat4 wvpMatrix;
	vec4 color;
};

layout(location = 0) in vec3 position;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
}

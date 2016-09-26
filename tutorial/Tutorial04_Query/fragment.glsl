// GLSL model shader

#version 140

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	vec4 color;
};

out vec4 fragColor;

void main()
{
	fragColor = color;
}

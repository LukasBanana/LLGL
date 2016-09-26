// GLSL model vertex shader

#version 140

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	vec4 color;
};

in vec3 position;

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
}

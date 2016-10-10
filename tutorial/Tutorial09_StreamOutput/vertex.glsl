// GLSL vertex shader
#version 150

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
};

in vec3 position;

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
}

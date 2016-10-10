// GLSL vertex shader
#version 150

in vec3 position;

void main()
{
	gl_Position = vec4(position, 1);
}

// GLSL shader version 1.30 (for OpenGL 3.1)
#version 130

in vec2 position;
in vec4 color;

out vec4 vColor;

void main()
{
	gl_Position = vec4(position, 0, 1);
	vColor = color;
}

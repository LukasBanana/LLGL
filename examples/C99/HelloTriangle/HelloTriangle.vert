// GLSL shader version 3.30 (for OpenGL 3.3)
#version 330

in vec2 position;
in vec4 color;

out vec4 vColor;

void main()
{
	gl_Position = vec4(position, 0, 1);
	vColor = color;
}

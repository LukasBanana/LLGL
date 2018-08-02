#version 330 core

in vec2 coord;
in vec4 color;

out vec4 vColor;

void main()
{
	gl_Position = vec4(coord, 0, 1);
	vColor      = color;
}

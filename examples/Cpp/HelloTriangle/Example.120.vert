// GLSL shader version 1.20 compatibility (for OpenGL 2.x)
#version 120

// Vertex attributes (these names must match our vertex format attributes)
attribute vec2 position;
attribute vec4 color;

// Vertex output to the fragment shader
varying vec4 vertexColor;

// Vertex shader main function
void main()
{
	gl_Position = vec4(position, 0, 1);
	vertexColor = color;
}

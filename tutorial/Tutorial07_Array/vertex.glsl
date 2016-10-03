// GLSL shader version 1.30 (for OpenGL 3.1)
#version 130

// Vertex attributes (these names must match our vertex format attributes)
in vec2 position;
in vec3 color;
in vec3 instanceColor;
in vec2 instanceOffset;
in float instanceScale;

// Vertex output to the fragment shader
out vec3 vertexColor;

// Vertex shader main function
void main()
{
	gl_Position = vec4(position * instanceScale + instanceOffset, 0, 1);
	vertexColor = instanceColor * color;
}

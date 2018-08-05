// GLSL shader version 4.50 (for Vulkan)
#version 450 core

// Vertex attributes (these names must match our vertex format attributes)
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

// Vertex output to the fragment shader
layout(location = 0) out vec3 vertexColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

// Vertex shader main function
void main()
{
	gl_Position = vec4(position, 0, 1);
	vertexColor = color;
}

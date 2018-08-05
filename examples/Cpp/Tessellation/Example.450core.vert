// GLSL vertex shader
#version 450

// Vertex attributes (these names must match out vertex format attributes)
layout(location = 0) in vec3 position;

// Vertex output to the fragment shader
layout(location = 0) out vec3 vPosition;

// Vertex shader main function
void main()
{
	// Pass vertex data to tessellation control shader
	vPosition = position;
}

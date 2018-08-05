// GLSL vertex shader
#version 400

// Vertex attributes (these names must match out vertex format attributes)
in vec3 position;

// Vertex output to the fragment shader
out vec3 vPosition;

// Vertex shader main function
void main()
{
	// Pass vertex data to tessellation control shader
	vPosition = position;
}

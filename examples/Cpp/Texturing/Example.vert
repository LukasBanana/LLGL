// GLSL texturing vertex shader
#version 140

// Vertex attributes (these names must match out vertex format attributes)
in vec2 position;
in vec2 texCoord;

// Vertex output to the fragment shader
out vec2 vTexCoord;

// Vertex shader main function
void main()
{
	// Pass vertex data to tessellation control shader
	gl_Position = vec4(position, 0, 1);
	vTexCoord = texCoord;
}

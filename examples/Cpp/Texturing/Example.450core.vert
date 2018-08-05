// GLSL texturing vertex shader
#version 450

// Vertex attributes (these names must match out vertex format attributes)
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

// Vertex output to the fragment shader
layout(location = 0) out vec2 vTexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

// Vertex shader main function
void main()
{
	// Pass vertex data to tessellation control shader
	gl_Position = vec4(position, 0, 1);
	vTexCoord = texCoord;
}

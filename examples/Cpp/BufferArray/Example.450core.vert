// GLSL shader version 4.50 (for Vulkan)
#version 450

// Vertex attributes (these names must match our vertex format attributes)
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 instanceColor;
layout(location = 3) in vec2 instanceOffset;
layout(location = 4) in float instanceScale;

// Vertex output to the fragment shader
layout(location = 0) out vec3 vertexColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

// Vertex shader main function
void main()
{
	gl_Position = vec4(position * instanceScale + instanceOffset, 0, 1);
	vertexColor = instanceColor * color;
}

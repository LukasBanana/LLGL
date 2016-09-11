// GLSL shader version 1.40 (for OpenGL 3.2)
#version 140

// Uniform buffer object (also named "Constant Buffer")
layout(std140) uniform Matrices
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 worldMatrix;
};

// Vertex attributes (these names must match out vertex format attributes)
in vec3 position;

// Vertex output to the fragment shader
out vec3 vertexColor;

// Vertex shader main function
void main()
{
	// Transform vertex by the world-view-projection matrix chain
	gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position, 1);
	vertexColor = (position.xyz + 1.0) * 0.5;
}

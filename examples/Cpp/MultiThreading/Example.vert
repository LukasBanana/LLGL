// GLSL vertex shader
#version 410

// Vertex attributes (these names must match our vertex format attributes)
in vec3 position;
in vec2 texCoord;

// Vertex output to the fragment shader
out vec3 vertexColor;

// Scene settings
layout(std140) uniform Scene
{
    mat4 wvpMatrix;
};

// Vertex shader main function
void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
	vertexColor = vec3(texCoord.x, texCoord.y, 1);
}

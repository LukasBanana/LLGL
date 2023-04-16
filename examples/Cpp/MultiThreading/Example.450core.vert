// GLSL vertex shader
#version 450 core

// Vertex attributes (these names must match our vertex format attributes)
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

// Vertex output to the fragment shader
layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vColor;

// Scene settings
layout(std140, binding = 1) uniform Scene
{
    mat4 wvpMatrix;
    mat4 wMatrix;
};

// Vertex shader main function
void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
    vNormal = normalize((wMatrix * vec4(normal, 0)).xyz);
	vColor = vec3(texCoord.x, texCoord.y, 1);
}

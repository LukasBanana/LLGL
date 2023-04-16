// GLSL vertex shader
#version 410

// Vertex attributes (these names must match our vertex format attributes)
in vec3 position;
in vec3 normal;
in vec2 texCoord;

// Vertex output to the fragment shader
out vec3 vNormal;
out vec3 vColor;

// Scene settings
layout(std140) uniform Scene
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

// GLSL vertex shader
#version 400 core

// Input constant data
layout(std140) uniform Scene
{
    mat4    vpMatrix;
    mat4    vMatrix;
    mat4    wMatrix;
    vec3    lightVec;
    float   texScale;
    float   tessLevelInner;
    float   tessLevelOuter;
    float   maxHeightFactor;
    float   shininessPower;
};

// Vertex attributes (these names must match the vertex format attributes)
in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texCoord;

// Vertex output to the tessellation-control shader
out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec2 vTexCoord;

// Vertex shader main function
void main()
{
	// Pass vertex data to tessellation control shader
    vWorldPos   = (wMatrix * vec4(position, 1)).xyz;
    vNormal     = normalize((wMatrix * vec4(normal, 0)).xyz);
    vTangent    = normalize((wMatrix * vec4(tangent, 0)).xyz);
    vBitangent  = normalize((wMatrix * vec4(bitangent, 0)).xyz);
    vTexCoord   = texCoord;
}

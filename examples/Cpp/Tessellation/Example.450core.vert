// GLSL vertex shader
#version 450 core

// Input constant data
layout(std140, binding = 1) uniform Scene
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
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texCoord;

// Vertex output to the tessellation-control shader
layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec3 vTangent;
layout(location = 3) out vec3 vBitangent;
layout(location = 4) out vec2 vTexCoord;

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

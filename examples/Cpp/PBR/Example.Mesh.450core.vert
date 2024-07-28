// PBR GLSL Mesh Vertex Shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4    cMatrix;
    mat4    vpMatrix;
    mat4    wMatrix;
    vec2    aspectRatio;
    float   mipCount;
    float   _pad0;
    vec4    lightDir;
    uint    skyboxLayer;
    uint    materialLayer;
    uvec2   _pad1;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texCoord;

layout(location = 0) out VMeshOut
{
    vec3 tangent;
    vec3 bitangent;
    vec3 normal;
    vec2 texCoord;
    vec4 worldPos;
}
outp;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outp.worldPos   = wMatrix * vec4(position, 1);
    gl_Position     = vpMatrix * outp.worldPos;
    outp.tangent    = normalize(wMatrix * vec4(tangent, 0)).xyz;
    outp.bitangent  = normalize(wMatrix * vec4(bitangent, 0)).xyz;
    outp.normal     = normalize(wMatrix * vec4(normal, 0)).xyz;
    outp.texCoord   = texCoord;
}



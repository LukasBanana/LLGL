// PBR GLSL Mesh Vertex Shader

#version 410 core

uniform Settings
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

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texCoord;

out VMeshOut
{
    vec3 tangent;
    vec3 bitangent;
    vec3 normal;
    vec2 texCoord;
    vec4 worldPos;
}
outp;

void main()
{
    outp.worldPos   = wMatrix * vec4(position, 1);
    gl_Position     = vpMatrix * outp.worldPos;
    outp.tangent    = normalize(wMatrix * vec4(tangent, 0)).xyz;
    outp.bitangent  = normalize(wMatrix * vec4(bitangent, 0)).xyz;
    outp.normal     = normalize(wMatrix * vec4(normal, 0)).xyz;
    outp.texCoord   = texCoord;
}



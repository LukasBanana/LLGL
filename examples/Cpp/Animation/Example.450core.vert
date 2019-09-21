// GLSL vertex shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4    wMatrix;
    mat4    vpMatrix;
    vec3    lightDir;
    float   shininess;
    vec4    viewPos;
    vec4    albedo;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 vWorldPos;
layout(location = 1) out vec4 vNormal;
layout(location = 2) out vec2 vTexCoord;

void main()
{
    vWorldPos   = wMatrix * vec4(position, 1);
    gl_Position = vpMatrix * vWorldPos;
    vNormal     = wMatrix * vec4(normal, 0);
    vTexCoord   = texCoord;
}

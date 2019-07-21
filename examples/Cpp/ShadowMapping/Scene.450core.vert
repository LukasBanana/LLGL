// GLSL scene vertex shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    mat4 vpShadowMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 vWorldPos;
layout(location = 1) out vec4 vNormal;

void main()
{
    vWorldPos = wMatrix * vec4(position, 1);
	gl_Position = vpMatrix * vWorldPos;
    vNormal = normalize(wMatrix * vec4(normal, 0));
}

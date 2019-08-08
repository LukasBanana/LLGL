// GLSL stencil vertex shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
}

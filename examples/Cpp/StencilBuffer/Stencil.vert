// GLSL stencil vertex shader

#version 330 core

layout(std140) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

in vec3 position;

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
}

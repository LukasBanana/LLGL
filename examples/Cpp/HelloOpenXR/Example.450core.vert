#version 450 core

layout(set = 0, binding = 0) uniform Globals
{
    mat4 viewProj;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vColor;

void main()
{
    gl_Position = viewProj * vec4(inPosition, 1.0);
    vColor      = inColor;
}

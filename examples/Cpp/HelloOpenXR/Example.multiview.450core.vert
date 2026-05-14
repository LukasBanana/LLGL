#version 450 core
#extension GL_EXT_multiview : require

layout(set = 0, binding = 0) uniform Globals
{
    mat4 viewProj[2];   // [0] = left eye, [1] = right eye
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vColor;

void main()
{
    gl_Position = viewProj[gl_ViewIndex] * vec4(inPosition, 1.0);
    vColor      = inColor;
}

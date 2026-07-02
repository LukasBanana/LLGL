#version 450 core

// Single-pass stereo (multiview) vertex shader. With VK_KHR_multiview the render pass broadcasts one draw
// to each view; gl_ViewIndex selects this invocation's view so the matching view-projection matrix is used.
#extension GL_EXT_multiview : require

layout(set = 0, binding = 0) uniform Globals
{
    mat4 viewProj[2];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vColor;

void main()
{
    gl_Position = viewProj[gl_ViewIndex] * vec4(inPosition, 1.0);
    vColor      = inColor;
}

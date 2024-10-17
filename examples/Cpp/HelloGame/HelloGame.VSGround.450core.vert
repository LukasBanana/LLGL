// GLSL shader for HelloGame example
// This file is part of the LLGL project

#version 450 core

#if GL_ES
precision mediump float;
#endif

layout(std140, binding = 1) uniform Scene
{
    mat4    vpMatrix;
    mat4    vpShadowMatrix;
    vec3    lightDir;
    float   shininess;
    vec3    viewPos;
    float   shadowSizeInv;
    vec3    warpCenter;
    float   warpIntensity;
    vec3    bendDir;
    float   ambientItensity;
    vec3    groundTint;
    float   groundScale;
    vec3    lightColor;
    float   warpScaleInv;
};


// VERTEX SHADER GROUND

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec3 vWorldPos;

void main()
{
    gl_Position = vpMatrix * vec4(position, 1);
    vTexCoord   = texCoord;
    vWorldPos   = position;
}


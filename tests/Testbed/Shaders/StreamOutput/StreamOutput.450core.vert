/*
 * StreamOutput.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef XFB_OUTPUT
#define XFB_OUTPUT 0
#endif

#if XFB_OUTPUT
layout(xfb_buffer = 0, xfb_stride = 40) out;
#endif

layout(std140, binding = 1) uniform SOScene
{
    mat4    vsMatrix;
    mat4    gsMatrices[3];
    vec4 	lightVec;
    float   normalizeFactorVS;
    float   normalizeFactorDS;
    float   tessLevelOuter;
    float   tessLevelInner;
};

layout(location = 0) in vec4 attribPosition;
layout(location = 1) in vec3 attribNormal;
layout(location = 2) in vec3 attribColor;

#if XFB_OUTPUT
out gl_PerVertex
{
    layout(xfb_offset = 0) vec4 gl_Position;
};
layout(location = 0, xfb_offset = 16) out vec3 normal;
layout(location = 1, xfb_offset = 28) out vec3 color;
#else
layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;
#endif

invariant gl_Position;

void main()
{
    vec4 normalizedPos = vec4(normalize(attribPosition.xyz), 1.0);
    gl_Position = vsMatrix * mix(attribPosition, normalizedPos, normalizeFactorVS);
    normal  = normalize(mix(attribNormal, normalizedPos.xyz, normalizeFactorVS));
    color   = attribColor;
}

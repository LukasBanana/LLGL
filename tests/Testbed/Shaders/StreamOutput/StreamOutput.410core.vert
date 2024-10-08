/*
 * StreamOutput.410core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 410 core

layout(std140) uniform SOScene
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

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;

void main()
{
    vec4 normalizedPos = vec4(normalize(attribPosition.xyz), 1.0);
    gl_Position = vsMatrix * mix(attribPosition, normalizedPos, normalizeFactorVS);
    normal  = normalize(mix(attribNormal, normalizedPos.xyz, normalizeFactorVS));
    color   = attribColor;
}

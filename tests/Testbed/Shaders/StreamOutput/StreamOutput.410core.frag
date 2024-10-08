/*
 * StreamOutput.410core.frag
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

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = max(0.0, dot(lightVec.xyz, normal));
    float shading = mix(0.2, 1.0, NdotL);
    fragColor = vec4(vColor * shading, 1.0);
}

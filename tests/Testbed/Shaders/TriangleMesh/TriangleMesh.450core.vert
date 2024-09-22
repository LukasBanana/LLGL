/*
 * TriangleMesh.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef ENABLE_TEXTURING
#define ENABLE_TEXTURING 0
#endif

layout(binding = 1, std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec3 vNormal;

#if ENABLE_TEXTURING
layout(location = 1) out vec2 vTexCoord;
#endif

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
    vNormal = normalize(wMatrix * vec4(normal, 0)).xyz;
    #if ENABLE_TEXTURING
    vTexCoord = texCoord;
    #endif
}


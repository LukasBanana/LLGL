/*
 * TriangleMesh.330core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

#ifndef ENABLE_TEXTURING
#define ENABLE_TEXTURING 0
#endif

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 vNormal;

#if ENABLE_TEXTURING
out vec2 vTexCoord;
#endif

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
    vNormal = normalize(wMatrix * vec4(normal, 0)).xyz;
    #if ENABLE_TEXTURING
    vTexCoord = texCoord;
    #endif
}


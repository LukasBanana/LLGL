/*
 * TriangleMesh.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef NUM_TEXTURES
#define NUM_TEXTURES 0
#endif

#ifndef IS_INSTANCED
#define IS_INSTANCED 0
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

#if IS_INSTANCED
layout(location = 3) in mat4 instanceMatrix;
layout(location = 7) in vec4 instanceColor;
#endif

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec4 vBaseColor;

#if NUM_TEXTURES != 0
layout(location = 2) out vec2 vTexCoord;
#endif

void main()
{
    #if IS_INSTANCED
    gl_Position = vpMatrix * (instanceMatrix * vec4(position, 1));
    vNormal = normalize(instanceMatrix * vec4(normal, 0)).xyz;
    vBaseColor = solidColor * instanceColor;
    #else
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
    vNormal = normalize(wMatrix * vec4(normal, 0)).xyz;
    vBaseColor = solidColor;
    #endif
    #if NUM_TEXTURES != 0
    vTexCoord = texCoord;
    #endif
}


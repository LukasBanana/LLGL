/*
 * ShadowMapping.VScene.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(binding = 1, std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    mat4 vpShadowMatrix;
    vec4 solidColor;
    vec4 lightVec;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 vWorldPos;
layout(location = 1) out vec4 vNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vWorldPos   = wMatrix * vec4(position, 1);
    gl_Position = vpMatrix * vWorldPos;
    vNormal     = normalize(wMatrix * vec4(normal, 0));
}


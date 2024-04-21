/*
 * SemanticErrors.VSMain.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

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

void main()
{
    gl_Position = wvpMatrix * vec4(position, 1); // <-- Expected error: Undefined identifier "wvpMatrix"
    vNormal = normalize(wMatrix * vec4(normal, 0)).xyz;
}


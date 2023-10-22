/*
 * DynamicTriangleMesh.330core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

layout(std140) uniform Scene
{
    mat4 vpMatrix;
};

uniform mat4 wMatrix;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
    vNormal = normalize(wMatrix * vec4(normal, 0)).xyz;
    vTexCoord = texCoord;
}


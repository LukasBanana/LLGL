/*
 * VertexFormats.330core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

#ifndef VERTEX_FORMAT
#define VERTEX_FORMAT 0
#endif

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
};

layout(location = 0) in vec2 position;
#if VERTEX_FORMAT == 1
layout(location = 1) in vec4 color;
#endif

out vec4 vColor;

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 0, 1));
    #if VERTEX_FORMAT == 1
    vColor = solidColor * color;
    #else
    vColor = solidColor;
    #endif
}


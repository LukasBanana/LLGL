/*
 * UnprojectedMesh.330core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

in vec2 position;
in vec4 color;

out vec4 vColor;

void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
}


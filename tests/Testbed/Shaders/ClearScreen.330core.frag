/*
 * ClearScreen.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

uniform vec4 clearColor;

out vec4 outColor;

void main()
{
    outColor = clearColor;
}

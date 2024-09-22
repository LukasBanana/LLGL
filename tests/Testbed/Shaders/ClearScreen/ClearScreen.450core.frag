/*
 * ClearScreen.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(push_constant) uniform Color
{
    vec4 clearColor;
};

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = clearColor;
}

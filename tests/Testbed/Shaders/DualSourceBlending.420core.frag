/*
 * DualSourceBlending.420core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 420 core

layout(location = 0) in vec2 vTexCoord;

layout(binding = 1) uniform sampler2D colorMapA;
layout(binding = 2) uniform sampler2D colorMapB;

layout(location = 0, index = 0) out vec4 outColorA;
layout(location = 0, index = 1) out vec4 outColorB;

void main()
{
    outColorA = texture(colorMapA, vTexCoord);
    outColorB = texture(colorMapB, vTexCoord);
}

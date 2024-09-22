/*
 * DualSourceBlending.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(location = 0) in vec2 vTexCoord;

layout(binding = 1) uniform texture2D colorMapA;
layout(binding = 2) uniform texture2D colorMapB;

layout(binding = 3) uniform sampler samplerA;
layout(binding = 4) uniform sampler samplerB;

layout(location = 0, index = 0) out vec4 outColorA;
layout(location = 0, index = 1) out vec4 outColorB;

void main()
{
    outColorA = texture(sampler2D(colorMapA, samplerA), vTexCoord);
    outColorB = texture(sampler2D(colorMapB, samplerB), vTexCoord);
}

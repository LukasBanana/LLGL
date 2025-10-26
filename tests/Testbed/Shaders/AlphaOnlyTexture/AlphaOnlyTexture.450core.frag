/*
 * AlphaOnlyTexture.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(location = 0) in vec2 vTexCoord;

layout(binding = 1) uniform texture2D colorMap;
layout(binding = 2) uniform sampler texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    float alpha = texture(sampler2D(colorMap, texSampler), vTexCoord).a;
    outColor = vec4(vec3(alpha), 1.0);
}

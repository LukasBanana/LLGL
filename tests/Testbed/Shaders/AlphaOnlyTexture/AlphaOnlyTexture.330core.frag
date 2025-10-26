/*
 * AlphaOnlyTexture.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

in vec2 vTexCoord;

uniform sampler2D colorMap;

out vec4 outColor;

void main()
{
    float alpha = texture(colorMap, vTexCoord).a;
    outColor = vec4(vec3(alpha), 1.0);
}

/*
 * CombinedSamplers.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D colorMapA_texSamplerA;
uniform sampler2D colorMapB_texSamplerA;
uniform sampler2D colorMapB_texSamplerB;
uniform sampler2D colorMapC_texSamplerB;

void main()
{
    vec4 col0 = texture(colorMapA_texSamplerA, vTexCoord);
    vec4 col1 = texture(colorMapB_texSamplerA, vTexCoord);
    vec4 col2 = texture(colorMapB_texSamplerB, vTexCoord);
    vec4 col3 = texture(colorMapC_texSamplerB, vTexCoord);

    vec2 blend = step(vec2(0.5), vTexCoord);

    outColor = mix(
        mix(col0, col1, blend.x),
        mix(col2, col3, blend.x),
        blend.y
    );
}

/*
 * DynamicTriangleMesh.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

uniform vec4 solidColor;
uniform vec3 lightVec;

in vec3 vNormal;
in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D colorMap;

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    vec4 albedo = texture(colorMap, vTexCoord);
    outColor = solidColor * albedo * vec4(vec3(shading), 1.0);
}

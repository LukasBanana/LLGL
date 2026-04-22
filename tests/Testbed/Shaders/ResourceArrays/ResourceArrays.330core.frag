/*
 * ResourceArrays.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

in vec3 vNormal;
in vec2 vTexCoord;

out vec4 outColor;

uniform sampler2D colorMaps[2];

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    vec4 albedo = texture(colorMaps[0], vTexCoord);
    vec4 detail = vec4(texture(colorMaps[1], vTexCoord * 0.25).rgb - vec3(0.5), 0.0);
    outColor = solidColor * (albedo + detail) * vec4(vec3(shading), 1.0);
}

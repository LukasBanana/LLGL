/*
 * SemanticErrors.PSMain.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(binding = 1, std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

layout(location = 0) in vec3 vNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    outColor = solidColor * vec4(vec3(shading), 1.0);
}

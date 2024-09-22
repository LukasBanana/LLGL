/*
 * DynamicTriangleMesh.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef ENABLE_SPIRV
#define ENABLE_SPIRV 0
#endif

layout(push_constant) uniform Model
{
    layout(offset = 64) vec4 solidColor;
    layout(offset = 80) vec3 lightVec;
};

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 outColor;

#if ENABLE_SPIRV
layout(binding = 3) uniform texture2D colorMap;
layout(binding = 4) uniform sampler linearSampler;
#else
layout(binding = 3) uniform sampler2D colorMap;
#endif

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    #if ENABLE_SPIRV
    vec4 albedo = texture(sampler2D(colorMap, linearSampler), vTexCoord);
    #else
    vec4 albedo = texture(colorMap, vTexCoord);
    #endif
    outColor = solidColor * albedo * vec4(vec3(shading), 1.0);
}

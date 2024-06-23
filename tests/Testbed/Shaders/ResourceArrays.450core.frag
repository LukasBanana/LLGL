/*
 * ResourceArrays.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef ENABLE_SPIRV
#define ENABLE_SPIRV 0
#endif

layout(binding = 1, std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 outColor;

#if ENABLE_SPIRV

layout(binding = 2) uniform texture2D colorMaps[2];
layout(binding = 4) uniform sampler texSamplers[2];

vec4 SampleTex(int index, vec2 texCoord)
{
    return texture(sampler2D(colorMaps[index], texSamplers[index]), texCoord);
}

#else

layout(binding = 2) uniform sampler2D colorMaps[2];

vec4 SampleTex(int index, vec2 texCoord)
{
    return texture(colorMaps[index], texCoord);
}

#endif

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    vec4 albedo = SampleTex(0, vTexCoord);
    vec4 detail = vec4(SampleTex(1, vTexCoord * 0.25).rgb - vec3(0.5), 0.0);
    outColor = solidColor * (albedo + detail) * vec4(vec3(shading), 1.0);
}

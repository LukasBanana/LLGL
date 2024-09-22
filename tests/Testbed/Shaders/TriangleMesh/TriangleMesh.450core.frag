/*
 * TriangleMesh.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef ENABLE_TEXTURING
#define ENABLE_TEXTURING 0
#endif

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

#if ENABLE_TEXTURING
layout(location = 1) in vec2 vTexCoord;
#endif

layout(location = 0) out vec4 outColor;

#if ENABLE_TEXTURING
#   if ENABLE_SPIRV
layout(binding = 2) uniform texture2D colorMap;
layout(binding = 3) uniform sampler linearSampler;
#   else
layout(binding = 2) uniform sampler2D colorMap;
#   endif
#endif

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    #if ENABLE_TEXTURING
    #   if ENABLE_SPIRV
    vec4 albedo = texture(sampler2D(colorMap, linearSampler), vTexCoord);
    #   else
    vec4 albedo = texture(colorMap, vTexCoord);
    #   endif
    #else
    vec4 albedo = vec4(1);
    #endif
    outColor = solidColor * albedo * vec4(vec3(shading), 1.0);
}

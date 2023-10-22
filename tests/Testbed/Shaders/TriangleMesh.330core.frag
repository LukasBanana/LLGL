/*
 * TriangleMesh.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

#ifndef ENABLE_TEXTURING
#define ENABLE_TEXTURING 0
#endif

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

in vec3 vNormal;

#if ENABLE_TEXTURING
in vec2 vTexCoord;
#endif

out vec4 outColor;

#if ENABLE_TEXTURING
uniform sampler2D colorMap;
#endif

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);
    #if ENABLE_TEXTURING
    vec4 albedo = texture(colorMap, vTexCoord);
    #else
    vec4 albedo = vec4(1);
    #endif
    outColor = solidColor * albedo * vec4(vec3(shading), 1.0);
}

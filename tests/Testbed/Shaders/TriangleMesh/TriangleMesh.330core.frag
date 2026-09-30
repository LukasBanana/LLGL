/*
 * TriangleMesh.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

#ifndef NUM_TEXTURES
#define NUM_TEXTURES 0
#endif

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    vec4 solidColor;
    vec3 lightVec;
};

in vec3 vNormal;
in vec4 vBaseColor;

#if NUM_TEXTURES != 0
in vec2 vTexCoord;
#endif

out vec4 outColor;

#if NUM_TEXTURES == 1

uniform sampler2D colorMap;

#elif NUM_TEXTURES == 8

uniform sampler2D colorMap0;
uniform sampler2D colorMap1;
uniform sampler2D colorMap2;
uniform sampler2D colorMap3;
uniform sampler2D colorMap4;
uniform sampler2D colorMap5;
uniform sampler2D colorMap6;
uniform sampler2D colorMap7;

#endif

void main()
{
    vec3 normal = normalize(vNormal);
    float NdotL = clamp(dot(lightVec, normal), 0.0, 1.0);
    float shading = mix(0.2, 1.0, NdotL);

    #if NUM_TEXTURES == 8

    // Multi texturing
    vec4 albedo = vec4(0);
    albedo += texture(colorMap0, vTexCoord);
    albedo += texture(colorMap1, vTexCoord);
    albedo += texture(colorMap2, vTexCoord);
    albedo += texture(colorMap3, vTexCoord);
    albedo += texture(colorMap4, vTexCoord);
    albedo += texture(colorMap5, vTexCoord);
    albedo += texture(colorMap6, vTexCoord);
    albedo += texture(colorMap7, vTexCoord);
    albedo /= float(NUM_TEXTURES);

    #elif NUM_TEXTURES == 1

    // Single texture
    vec4 albedo = texture(colorMap, vTexCoord);

    #else

    // Solid color only
    vec4 albedo = vec4(1);

    #endif

    outColor = vBaseColor * albedo * vec4(vec3(shading), 1.0);
}

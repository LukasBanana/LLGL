/*
 * TriangleMesh.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef NUM_TEXTURES
#define NUM_TEXTURES 0
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

#if NUM_TEXTURES != 0
layout(location = 1) in vec2 vTexCoord;
#endif

layout(location = 0) out vec4 outColor;

#if NUM_TEXTURES == 8

#   if ENABLE_SPIRV
layout(binding = 2) uniform texture2D colorMap0;
layout(binding = 3) uniform texture2D colorMap1;
layout(binding = 4) uniform texture2D colorMap2;
layout(binding = 5) uniform texture2D colorMap3;
layout(binding = 6) uniform texture2D colorMap4;
layout(binding = 7) uniform texture2D colorMap5;
layout(binding = 8) uniform texture2D colorMap6;
layout(binding = 9) uniform texture2D colorMap7;
layout(binding = 10) uniform sampler texSampler0;
layout(binding = 11) uniform sampler texSampler1;
#   else
layout(binding = 2) uniform sampler2D colorMap0;
layout(binding = 3) uniform sampler2D colorMap1;
layout(binding = 4) uniform sampler2D colorMap2;
layout(binding = 5) uniform sampler2D colorMap3;
layout(binding = 6) uniform sampler2D colorMap4;
layout(binding = 7) uniform sampler2D colorMap5;
layout(binding = 8) uniform sampler2D colorMap6;
layout(binding = 9) uniform sampler2D colorMap7;
#   endif

#elif NUM_TEXTURES == 1

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

    #if NUM_TEXTURES == 8
    
    // Multi texturing
    vec4 albedo = vec4(0);
    #   if ENABLE_SPIRV
    albedo += texture(sampler2D(colorMap0, texSampler0), vTexCoord);
    albedo += texture(sampler2D(colorMap1, texSampler1), vTexCoord);
    albedo += texture(sampler2D(colorMap2, texSampler0), vTexCoord);
    albedo += texture(sampler2D(colorMap3, texSampler1), vTexCoord);
    albedo += texture(sampler2D(colorMap4, texSampler0), vTexCoord);
    albedo += texture(sampler2D(colorMap5, texSampler1), vTexCoord);
    albedo += texture(sampler2D(colorMap6, texSampler0), vTexCoord);
    albedo += texture(sampler2D(colorMap7, texSampler1), vTexCoord);
    #   else
    albedo += texture(colorMap0, vTexCoord);
    albedo += texture(colorMap1, vTexCoord);
    albedo += texture(colorMap2, vTexCoord);
    albedo += texture(colorMap3, vTexCoord);
    albedo += texture(colorMap4, vTexCoord);
    albedo += texture(colorMap5, vTexCoord);
    albedo += texture(colorMap6, vTexCoord);
    albedo += texture(colorMap7, vTexCoord);
    #   endif
    albedo /= float(NUM_TEXTURES);

    #elif NUM_TEXTURES == 1

    // Single texture
    #   if ENABLE_SPIRV
    vec4 albedo = texture(sampler2D(colorMap, linearSampler), vTexCoord);
    #   else
    vec4 albedo = texture(colorMap, vTexCoord);
    #   endif

    #else

    // Solid color only
    vec4 albedo = vec4(1);

    #endif

    outColor = solidColor * albedo * vec4(vec3(shading), 1.0);
}

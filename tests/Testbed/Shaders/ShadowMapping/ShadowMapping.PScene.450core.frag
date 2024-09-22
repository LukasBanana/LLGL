/*
 * ShadowMapping.PScene.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(binding = 1, std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    mat4 vpShadowMatrix;
    vec4 solidColor;
    vec4 lightVec;
};

layout(location = 0) in vec4 vWorldPos;
layout(location = 1) in vec4 vNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform texture2D shadowMap;
layout(binding = 3) uniform samplerShadow shadowSampler;

void main()
{
    // Project world position into shadow-map space
    vec4 shadowPos = vpShadowMatrix * vWorldPos;
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * vec2(0.5, -0.5) + 0.5;

    // Sample shadow map
    float shadow = texture(sampler2DShadow(shadowMap, shadowSampler), shadowPos.xyz);

    // Compute lighting
    vec3 normal = normalize(vNormal.xyz);
    float NdotL = max(0.2, dot(lightVec.xyz, normal));
    
    // Set final output color
    shadow = mix(0.2, 1.0, shadow);
    outColor = solidColor * vec4(vec3(NdotL * shadow), 1.0);
}

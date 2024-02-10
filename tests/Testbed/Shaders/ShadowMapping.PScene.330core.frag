/*
 * ShadowMapping.PScene.330core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    mat4 vpShadowMatrix;
    vec4 solidColor;
    vec4 lightVec;
};

in vec4 vWorldPos;
in vec4 vNormal;

out vec4 outColor;

uniform sampler2DShadow shadowMap;

void main()
{
    // Project world position into shadow-map space
    vec4 shadowPos = vpShadowMatrix * vWorldPos;
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * vec2(0.5, -0.5) + 0.5;
    shadowPos.z = shadowPos.z * 0.5 + 0.5; // Transform from unit-cube NDC [-1,+1] to [0,1] depth range

    // Sample shadow map
    float shadow = texture(shadowMap, shadowPos.xyz);

    // Compute lighting
    vec3 normal = normalize(vNormal.xyz);
    float NdotL = max(0.2, dot(lightVec.xyz, normal));
    
    // Set final output color
    shadow = mix(0.2, 1.0, shadow);
    outColor = solidColor * vec4(vec3(NdotL * shadow), 1.0);
}

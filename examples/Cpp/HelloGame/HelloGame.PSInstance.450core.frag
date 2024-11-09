// GLSL shader for HelloGame example
// This file is part of the LLGL project

#version 450 core

#ifndef ENABLE_SPIRV
#define ENABLE_SPIRV 0
#endif

layout(std140, binding = 1) uniform Scene
{
    mat4    vpMatrix;
    mat4    vpShadowMatrix;
    vec3    lightDir;
    float   shininess;
    vec3    viewPos;
    float   shadowSizeInv;
    vec3    warpCenter;
    float   warpIntensity;
    vec3    bendDir;
    float   ambientItensity;
    vec3    groundTint;
    float   groundScale;
    vec3    lightColor;
    float   warpScaleInv;
};


// PIXEL SHADER INSTANCE

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec4 vColor;

layout(location = 0) out vec4 outColor;

#if ENABLE_SPIRV
layout(binding = 4) uniform texture2D shadowMap;
layout(binding = 5) uniform samplerShadow shadowMapSampler;
#else
layout(binding = 4) uniform sampler2DShadow shadowMap;
#endif

float SampleShadowMapOffset(vec3 worldPos, vec2 offset)
{
    // Project world position into shadow-map space
    vec4 shadowPos = vpShadowMatrix * vec4(worldPos, 1);
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * vec2(0.5, -0.5) + 0.5 + offset * shadowSizeInv;

    #if !ENABLE_SPIRV
    shadowPos.z = shadowPos.z * 0.5 + 0.5; // OpenGL NDC space adjustment
    #endif
    
    // Sample shadow map
    #if ENABLE_SPIRV
    return texture(sampler2DShadow(shadowMap, shadowMapSampler), shadowPos.xyz);
    #else
    return texture(shadowMap, shadowPos.xyz);
    #endif
}

float SampleShadowMapPCF(vec2 screenPos, vec3 worldPos)
{
    // Perform percentage closer filtering (PCF) as described here:
    // https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
    vec2 offset = vec2(greaterThan(fract(screenPos * 0.5), vec2(0.25)));
    if (offset.y > 1.1)
        offset.y = 0.0;

    float shadowSamples =
    (
        SampleShadowMapOffset(worldPos, offset + vec2(-1.5, +0.5)) +
        SampleShadowMapOffset(worldPos, offset + vec2(+0.5, +0.5)) +
        SampleShadowMapOffset(worldPos, offset + vec2(-1.5, -1.5)) +
        SampleShadowMapOffset(worldPos, offset + vec2(+0.5, -1.5))
    );

    return shadowSamples * 0.25;
}

void main()
{
    // Get input color
    vec4    albedo      = vColor;

    // Diffuse lighting
    vec3    lightVec    = -lightDir.xyz;
    vec3    normal      = normalize(vNormal);
    float   NdotL       = mix(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    vec3    diffuse     = albedo.rgb * NdotL;

    // Specular lighting
    vec3    viewDir     = normalize(viewPos - vWorldPos);
    vec3    halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    vec3    specular    = vec3(pow(max(0.0, NdotH), shininess));

    // Apply shadow mapping
    float   shadow      = mix(ambientItensity, 1.0, SampleShadowMapPCF(gl_FragCoord.xy, vWorldPos));

    // Set final output color
    vec3    light       = lightColor * (diffuse + specular) * shadow;
    outColor = vec4(light, albedo.a);
}


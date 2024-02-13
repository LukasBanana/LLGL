/*
 * ShadowMapping.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

struct Scene
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float4x4 vpShadowMatrix;
    float4   solidColor;
    float4   lightVec;
};

struct VertexIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
};

struct VertexOut
{
    float4 position [[position]];
    float4 worldPos;
    float4 normal;
};

vertex float4 VShadow(
    VertexIn        inp         [[stage_in]],
    constant Scene& scene       [[buffer(1)]])
{
	return scene.vpShadowMatrix * (scene.wMatrix * float4(inp.position, 1));
}

vertex VertexOut VScene(
    VertexIn        inp     [[stage_in]],
    constant Scene& scene   [[buffer(1)]])
{
    VertexOut outp;
    outp.worldPos   = scene.wMatrix * float4(inp.position, 1);
    outp.position   = scene.vpMatrix * (scene.wMatrix * float4(inp.position, 1));
    outp.normal     = normalize(scene.wMatrix * float4(inp.normal, 0));
    return outp;
}

fragment float4 PScene(
    VertexOut       inp             [[stage_in]],
    constant Scene& scene           [[buffer(1)]],
    depth2d<float>  shadowMap       [[texture(2)]],
    sampler         shadowSampler   [[sampler(3)]])
{
    // Project world position into shadow-map space
    float4 shadowPos = scene.vpShadowMatrix * inp.worldPos;
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;

    // Sample shadow map
    float shadow = shadowMap.sample_compare(shadowSampler, shadowPos.xy, shadowPos.z);

    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(scene.lightVec.xyz, normal));
    
    // Set final output color
    shadow = mix(0.2, 1.0, shadow);
    return scene.solidColor * float4((float3)(NdotL * shadow), 1.0);
}

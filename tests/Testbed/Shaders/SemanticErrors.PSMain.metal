/*
 * SemanticErrors.PSMain.metal
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
    float4   solidColor;
    float3   lightVec;
};

struct VertexOut
{
    float4 position [[position]];
    float3 normal;
};

fragment float4 PSMain(
    VertexOut inp [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(scene.lightVec, normal));
    float shading = mix(0.2, 1.0, NdotL);
    return scene.solidColor * float4((float3)shading, 1.0);
}

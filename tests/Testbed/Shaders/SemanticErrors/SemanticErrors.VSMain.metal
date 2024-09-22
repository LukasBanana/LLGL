/*
 * SemanticErrors.VSMain.metal
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

struct VertexIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 normal;
};

vertex VertexOut VSMain(
    VertexIn inp [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    VertexOut outp;
    outp.position = scene.wvpMatrix * float4(inp.position, 1); // <-- Expected error: Undefined identifier "wvpMatrix"
    outp.normal   = normalize(scene.wMatrix * float4(inp.normal, 0)).xyz;
    return outp;
}

/*
 * TriangleMesh.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

#ifndef ENABLE_TEXTURING
#define ENABLE_TEXTURING 0
#endif

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
    #if ENABLE_TEXTURING
    float2 texCoord;
    #endif
};

vertex VertexOut VSMain(
    VertexIn inp [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    VertexOut outp;
    outp.position = scene.vpMatrix * (scene.wMatrix * float4(inp.position, 1));
    outp.normal   = normalize(scene.wMatrix * float4(inp.normal, 0)).xyz;
    #if ENABLE_TEXTURING
    outp.texCoord = inp.texCoord;
    #endif
    return outp;
}

fragment float4 PSMain(
    VertexOut inp [[stage_in]],
    #if ENABLE_TEXTURING
    texture2d<float> colorMap [[texture(2)]],
    sampler linearSampler [[sampler(3)]],
    #endif
    constant Scene& scene [[buffer(1)]])
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(scene.lightVec, normal));
    float shading = mix(0.2, 1.0, NdotL);
    #if ENABLE_TEXTURING
    float4 albedo = colorMap.sample(linearSampler, inp.texCoord);
    #else
    float4 albedo = (float4)1;
    #endif
    return scene.solidColor * albedo * float4((float3)shading, 1.0);
}

/*
 * DynamicTriangleMesh.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

struct Scene
{
    float4x4 vpMatrix;
};

struct Model
{
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
    float2 texCoord;
};

vertex VertexOut VSMain(
    VertexIn        inp   [[stage_in]],
    constant Scene& scene [[buffer(1)]],
    constant Model& model [[buffer(2)]])
{
    VertexOut outp;
    outp.position = scene.vpMatrix * (model.wMatrix * float4(inp.position, 1));
    outp.normal   = normalize(model.wMatrix * float4(inp.normal, 0)).xyz;
    outp.texCoord = inp.texCoord;
    return outp;
}

fragment float4 PSMain(
    VertexOut        inp           [[stage_in]],
    texture2d<float> colorMap      [[texture(3)]],
    sampler          linearSampler [[sampler(4)]],
    constant Model&  model         [[buffer(2)]])
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(model.lightVec, normal));
    float shading = mix(0.2, 1.0, NdotL);
    float4 albedo = colorMap.sample(linearSampler, inp.texCoord);
    return model.solidColor * albedo * float4((float3)shading, 1.0);
}

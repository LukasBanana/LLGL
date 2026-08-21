/*
 * TriangleMesh.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

#ifndef NUM_TEXTURES
#define NUM_TEXTURES 0
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
    #if NUM_TEXTURES != 0
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
    #if NUM_TEXTURES != 0
    outp.texCoord = inp.texCoord;
    #endif
    return outp;
}

fragment float4 PSMain(
    VertexOut inp [[stage_in]],

    #if NUM_TEXTURES == 8

    texture2d<float> colorMap0 [[texture(2)]],
    texture2d<float> colorMap1 [[texture(3)]],
    texture2d<float> colorMap2 [[texture(4)]],
    texture2d<float> colorMap3 [[texture(5)]],
    texture2d<float> colorMap4 [[texture(6)]],
    texture2d<float> colorMap5 [[texture(7)]],
    texture2d<float> colorMap6 [[texture(8)]],
    texture2d<float> colorMap7 [[texture(9)]],

    sampler texSampler0 [[sampler(10)]],
    sampler texSampler1 [[sampler(11)]],

    #elif NUM_TEXTURES == 1

    texture2d<float> colorMap [[texture(2)]],
    sampler linearSampler [[sampler(3)]],

    #endif

    constant Scene& scene [[buffer(1)]])
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(scene.lightVec, normal));
    float shading = mix(0.2, 1.0, NdotL);

    #if NUM_TEXTURES == 8

    // Multi texturing
    float4 albedo = (float4)0;
    albedo += colorMap0.sample(texSampler0, inp.texCoord);
    albedo += colorMap1.sample(texSampler1, inp.texCoord);
    albedo += colorMap2.sample(texSampler0, inp.texCoord);
    albedo += colorMap3.sample(texSampler1, inp.texCoord);
    albedo += colorMap4.sample(texSampler0, inp.texCoord);
    albedo += colorMap5.sample(texSampler1, inp.texCoord);
    albedo += colorMap6.sample(texSampler0, inp.texCoord);
    albedo += colorMap7.sample(texSampler1, inp.texCoord);
    albedo /= NUM_TEXTURES;

    #elif NUM_TEXTURES == 1

    // Single texture
    float4 albedo = colorMap.sample(linearSampler, inp.texCoord);

    #else

    // Solid color only
    float4 albedo = (float4)1;

    #endif

    return scene.solidColor * albedo * float4((float3)shading, 1.0);
}

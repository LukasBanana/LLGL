/*
 * TriangleMesh.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef ENABLE_TEXTURING
#define ENABLE_TEXTURING 0
#endif

cbuffer Scene : register(b1)
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float4   solidColor;
    float3   lightVec;
}

struct VertexIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    #if ENABLE_TEXTURING
    float2 texCoord : TEXCOORD;
    #endif
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    outp.position = mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
    outp.normal   = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
    #if ENABLE_TEXTURING
    outp.texCoord = inp.texCoord;
    #endif
}

#if ENABLE_TEXTURING
Texture2D colorMap : register(t2);
SamplerState linearSampler : register(s3);
#endif

float4 PSMain(VertexOut inp) : SV_Target
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(lightVec, normal));
    float shading = lerp(0.2, 1.0, NdotL);
    #if ENABLE_TEXTURING
    float4 albedo = colorMap.Sample(linearSampler, inp.texCoord);
    #else
    float4 albedo = (float4)1;
    #endif
    return solidColor * albedo * float4((float3)shading, 1.0);
}

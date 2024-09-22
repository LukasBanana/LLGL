/*
 * ResourceArrays.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

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
    float2 texCoord : TEXCOORD;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    outp.position = mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
    outp.normal   = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
    outp.texCoord = inp.texCoord;
}

Texture2D colorMaps[2] : register(t2);
SamplerState texSamplers[2] : register(s4);

float4 PSMain(VertexOut inp) : SV_Target
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(lightVec, normal));
    float shading = lerp(0.2, 1.0, NdotL);
    float4 albedo = colorMaps[0].Sample(texSamplers[0], inp.texCoord);
    float4 detail = float4(colorMaps[1].Sample(texSamplers[1], inp.texCoord * 0.25).rgb - (float3)0.5, 0.0);
    return solidColor * (albedo + detail) * float4((float3)shading, 1.0);
}

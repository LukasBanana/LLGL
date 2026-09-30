/*
 * TriangleMesh.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef NUM_TEXTURES
#define NUM_TEXTURES 0
#endif

#ifndef IS_INSTANCED
#define IS_INSTANCED 0
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
    float3      position        : POSITION;
    float3      normal          : NORMAL;
    float2      texCoord        : TEXCOORD;
    #if IS_INSTANCED
    float4x4    instanceMatrix  : INSTANCEMATRIX;
    float4      instanceColor   : INSTANCECOLOR;
    #endif
};

struct VertexOut
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    #if NUM_TEXTURES > 0
    float2 texCoord : TEXCOORD;
    #endif
    float4 baseColor : BASECOLOR;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    #if IS_INSTANCED
    outp.position   = mul(vpMatrix, mul(inp.instanceMatrix, float4(inp.position, 1)));
    outp.normal     = normalize(mul(inp.instanceMatrix, float4(inp.normal, 0)).xyz);
    outp.baseColor  = solidColor * inp.instanceColor;
    #else
    outp.position   = mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
    outp.normal     = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
    outp.baseColor  = solidColor;
    #endif
    #if NUM_TEXTURES > 0
    outp.texCoord = inp.texCoord;
    #endif
}

#if NUM_TEXTURES == 1

Texture2D colorMap : register(t2);
SamplerState linearSampler : register(s3);

#elif NUM_TEXTURES == 8

Texture2D colorMap0 : register(t2);
Texture2D colorMap1 : register(t3);
Texture2D colorMap2 : register(t4);
Texture2D colorMap3 : register(t5);
Texture2D colorMap4 : register(t6);
Texture2D colorMap5 : register(t7);
Texture2D colorMap6 : register(t8);
Texture2D colorMap7 : register(t9);

SamplerState texSampler0 : register(s10);
SamplerState texSampler1 : register(s11);

#endif

float4 PSMain(VertexOut inp) : SV_Target
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(lightVec, normal));
    float shading = lerp(0.2, 1.0, NdotL);

    #if NUM_TEXTURES == 8
    
    // Multi texturing
    float4 albedo = (float4)0;
    albedo += colorMap0.Sample(texSampler0, inp.texCoord);
    albedo += colorMap1.Sample(texSampler1, inp.texCoord);
    albedo += colorMap2.Sample(texSampler0, inp.texCoord);
    albedo += colorMap3.Sample(texSampler1, inp.texCoord);
    albedo += colorMap4.Sample(texSampler0, inp.texCoord);
    albedo += colorMap5.Sample(texSampler1, inp.texCoord);
    albedo += colorMap6.Sample(texSampler0, inp.texCoord);
    albedo += colorMap7.Sample(texSampler1, inp.texCoord);
    albedo /= NUM_TEXTURES;

    #elif NUM_TEXTURES == 1

    // Single texture
    float4 albedo = colorMap.Sample(linearSampler, inp.texCoord);

    #else

    // Solid color only
    float4 albedo = (float4)1;

    #endif

    return inp.baseColor * albedo * float4((float3)shading, 1.0);
}

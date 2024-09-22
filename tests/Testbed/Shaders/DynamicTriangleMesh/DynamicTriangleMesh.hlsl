/*
 * DynamicTriangleMesh.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer Scene : register(b1)
{
    float4x4 vpMatrix;
}

// If this is register b0, it might overlap with "$Globals" cbuffer in the pixel shader (depending on shader optimization)
cbuffer Model : register(b2)
{
    float4x4 wMatrix;
    float4   solidColor;
}

float3 lightVec; // Implicitly in $Globals cbuffer

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

Texture2D colorMap : register(t3);
SamplerState linearSampler : register(s4);

float4 PSMain(VertexOut inp) : SV_Target
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(lightVec, normal));
    float shading = lerp(0.2, 1.0, NdotL);
    float4 albedo = colorMap.Sample(linearSampler, inp.texCoord);
    return solidColor * albedo * float4((float3)shading, 1.0);
}

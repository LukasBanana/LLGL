/*
 * SemanticErrors.PSMain.hlsl
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

struct VertexOut
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
};

float4 PSMain(VertexOut inp) : SV_Target
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(lightVec, normal));
    float shading = lerp(0.2, 1.0, NdotL);
    return solidColor * float4((float3)shading, 1.0);
}

/*
 * ShadowMapping.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer Scene : register(b1)
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float4x4 vpShadowMatrix;
    float4   solidColor;
    float4   lightVec;
}

struct VertexIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 worldPos : WORLDPOS;
    float4 normal   : NORMAL;
};

float4 VShadow(VertexIn inp) : SV_Position
{
	return mul(vpShadowMatrix, mul(wMatrix, float4(inp.position, 1)));
}

void VScene(VertexIn inp, out VertexOut outp)
{
    outp.worldPos   = mul(wMatrix, float4(inp.position, 1));
    outp.position   = mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
    outp.normal     = normalize(mul(wMatrix, float4(inp.normal, 0)));
}

Texture2D shadowMap : register(t2);
SamplerComparisonState shadowSampler : register(s3);

float4 PScene(VertexOut inp) : SV_Target
{
    // Project world position into shadow-map space
    float4 shadowPos = mul(vpShadowMatrix, inp.worldPos);
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;

    // Sample shadow map
    float shadow = shadowMap.SampleCmp(shadowSampler, shadowPos.xy, shadowPos.z);

    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(lightVec.xyz, normal));
    
    // Set final output color
    shadow = lerp(0.2, 1.0, shadow);
    return solidColor * float4((float3)(NdotL * shadow), 1.0);
}

/*
 * SemanticErrors.VSMain.hlsl
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
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    outp.position = mul(wvpMatrix, float4(inp.position, 1)); // <-- Expected error: Undefined identifier "wvpMatrix"
    outp.normal   = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
}

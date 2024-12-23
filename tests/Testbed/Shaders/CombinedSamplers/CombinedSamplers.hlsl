/*
 * CombinedSamplers.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer Scene : register(b1)
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
}

struct VertexIn
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    outp.position = mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
    outp.texCoord = inp.texCoord;
}

Texture2D colorMapA : register(t2);
Texture2D colorMapB : register(t3);
Texture2D colorMapC : register(t4);

SamplerState texSamplerA : register(s5);
SamplerState texSamplerB : register(s6);

float4 PSMain(VertexOut inp) : SV_Target
{
    float4 col0 = colorMapA.Sample(texSamplerA, inp.texCoord);
    float4 col1 = colorMapB.Sample(texSamplerA, inp.texCoord);
    float4 col2 = colorMapB.Sample(texSamplerB, inp.texCoord);
    float4 col3 = colorMapC.Sample(texSamplerB, inp.texCoord);

    float2 blend = step((float2)0.5, inp.texCoord);

    return lerp(
        lerp(col0, col1, blend.x),
        lerp(col2, col3, blend.x),
        blend.y
    );
}

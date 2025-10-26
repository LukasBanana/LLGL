/*
 * AlphaOnlyTexture.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

void VSMain(uint id : SV_VertexID, out VertexOut outp)
{
	outp.position = float4(id == 2 ? 3.0 : -1.0, id == 0 ? 3.0 : -1.0, 1.0, 1.0);
	outp.texCoord = outp.position.xy * float2(0.5, -0.5) + 0.5;
}

Texture2D<float4> colorMap : register(t1);
SamplerState texSampler : register(s2);

float4 PSMain(VertexOut inp) : SV_Target
{
    float alpha = colorMap.Sample(texSampler, inp.texCoord).a;
    return float4((float3)alpha, 1.0);
}

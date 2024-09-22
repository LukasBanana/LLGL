/*
 * DualSourceBlending.hlsl
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

Texture2D colorMapA : register(t1);
Texture2D colorMapB : register(t2);
SamplerState samplerA : register(s3);
SamplerState samplerB : register(s4);

void PSMain(
    in VertexOut inp,
    out float4 outColorA : SV_Target0,
    out float4 outColorB : SV_Target1)
{
    outColorA = colorMapA.Sample(samplerA, inp.texCoord);
    outColorB = colorMapB.Sample(samplerB, inp.texCoord);
}

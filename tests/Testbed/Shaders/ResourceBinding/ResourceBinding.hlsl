/*
 * ResourceBinding.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

Buffer<int4> inBufferA : register(t0);
Buffer<int4> inBufferB : register(t1);
RWBuffer<int4> outBufferA : register(u2);
RWBuffer<int4> outBufferB : register(u4);

Texture1D<int4> inTextureA : register(t2);
Texture1D<int4> inTextureB : register(t4);
RWTexture1D<int4> outTextureA : register(u0);
RWTexture1D<int4> outTextureB : register(u1);

[numthreads(1, 1, 1)]
void CSMain(uint id : SV_DispatchThreadID)
{
    outBufferA[id] = inBufferA[id] + inBufferB[id];
    outBufferB[id] = (inBufferA[id] - inBufferB[id]) * 2;

    outTextureA[id] = inTextureA.Load((int)id) + inTextureB.Load((int)id);
    outTextureB[id] = (inTextureA.Load((int)id) - inTextureB.Load((int)id)) * 2;
}

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

void VSMain(uint id : SV_VertexID, out VertexOut outp)
{
	outp.position = float4(0.0, 0.0, 0.0, 1.0); //float4(id == 2 ? 3.0 : -1.0, id == 0 ? 3.0 : -1.0, 1.0, 1.0);
	outp.texCoord = float2(0.0, 0.0); //outp.position.xy * float2(0.5, -0.5) + 0.5;

    outBufferA[id] = (inBufferA[id] + inBufferB[id]) * 3;
}

void PSMain(
    in VertexOut inp,
    out int4 outColorA : SV_Target0,
    out int4 outColorB : SV_Target1)
{
    int id = (int)inp.texCoord.x;

    outBufferB[id] = (inBufferA[id] - inBufferB[id]) / 2;

    outColorA = inTextureA.Load((int)id) + inTextureB.Load((int)id);
    outColorB = (inTextureA.Load((int)id) - inTextureB.Load((int)id)) * 2;
}
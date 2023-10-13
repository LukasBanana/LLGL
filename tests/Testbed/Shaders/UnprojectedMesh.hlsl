/*
 * UnprojectedMesh.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

struct VertexIn
{
    float2 position : POSITION;
    float4 color    : COLOR;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    outp.position = float4(inp.position, 0, 1);
    outp.color    = inp.color;
}

float4 PSMain(VertexOut inp) : SV_Target
{
    return inp.color;
}

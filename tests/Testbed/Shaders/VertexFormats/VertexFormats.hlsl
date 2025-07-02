/*
 * VertexFormats.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef VERTEX_FORMAT
#define VERTEX_FORMAT 0
#endif

cbuffer Scene : register(b1)
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float4   solidColor;
}

struct VertexIn
{
    float2 position : POSITION;
    #if VERTEX_FORMAT == 1
    float4 color    : COLOR;
    #endif
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    outp.position = mul(vpMatrix, mul(wMatrix, float4(inp.position, 0, 1)));
    #if VERTEX_FORMAT == 1
    outp.color = solidColor * inp.color;
    #else
    outp.color = solidColor;
    #endif
}

float4 PSMain(VertexOut inp) : SV_Target
{
    return inp.color;
}

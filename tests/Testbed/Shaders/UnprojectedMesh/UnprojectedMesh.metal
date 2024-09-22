/*
 * UnprojectedMesh.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

struct VertexIn
{
    float2 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
};

struct VertexOut
{
    float4 position [[position]];
    float4 color;
};

vertex VertexOut VSMain(VertexIn inp [[stage_in]])
{
    VertexOut outp;
    outp.position = float4(inp.position, 0, 1);
    outp.color    = inp.color;
    return outp;
}

fragment float4 PSMain(VertexOut inp [[stage_in]])
{
    return inp.color;
}

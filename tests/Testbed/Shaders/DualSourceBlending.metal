/*
 * DualSourceBlending.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
};

vertex VertexOut VSMain(uint id [[vertex_id]])
{
    VertexOut outp;
	outp.position = float4(id == 2 ? 3.0 : -1.0, id == 0 ? 3.0 : -1.0, 1.0, 1.0);
	outp.texCoord = outp.position.xy * float2(0.5, -0.5) + 0.5;
    return outp;
}

struct PixelOut
{
    float4 colorA [[color(0), index(0)]];
    float4 colorB [[color(0), index(1)]];
};

fragment PixelOut PSMain(
    VertexOut        inp       [[stage_in]],
    texture2d<float> colorMapA [[texture(1)]],
    texture2d<float> colorMapB [[texture(2)]],
    sampler          samplerA  [[sampler(3)]],
    sampler          samplerB  [[sampler(4)]])
{
    PixelOut outp;
    outp.colorA = colorMapA.sample(samplerA, inp.texCoord);
    outp.colorB = colorMapB.sample(samplerB, inp.texCoord);
    return outp;
}

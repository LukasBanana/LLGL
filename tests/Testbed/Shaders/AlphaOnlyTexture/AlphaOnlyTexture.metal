/*
 * AlphaOnlyTexture.metal
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

fragment float4 PSMain(
    VertexOut        inp        [[stage_in]],
    texture2d<float> colorMap   [[texture(1)]],
    sampler          texSampler [[sampler(2)]])
{
    float alpha = colorMap.sample(texSampler, inp.texCoord).a;
    return float4((float3)alpha, 1.0);
}

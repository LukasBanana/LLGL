/*
 * VertexFormats.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

#ifndef VERTEX_FORMAT
#define VERTEX_FORMAT 0
#endif

struct Scene
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float4   solidColor;
};

struct VertexIn
{
    float2 position [[attribute(0)]];
    #if VERTEX_FORMAT == 1
    float4 color    [[attribute(1)]];
    #endif
};

struct VertexOut
{
    float4 position [[position]];
    float4 color;
};

vertex VertexOut VSMain(
    VertexIn        inp   [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    VertexOut outp;
    outp.position = scene.vpMatrix * (scene.wMatrix * float4(inp.position, 0, 1));
    #if VERTEX_FORMAT == 1
    outp.color = inp.color;
    #else
    outp.color = scene.solidColor;
    #endif
    return outp;
}

fragment float4 PSMain(VertexOut inp [[stage_in]])
{
    return inp.color;
}

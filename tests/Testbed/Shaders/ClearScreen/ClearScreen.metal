/*
 * ClearScreen.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

vertex float4 VSMain(uint id [[vertex_id]])
{
	return float4(
        id == 1 ? 3.0 : -1.0,
        id == 2 ? -3.0 : 1.0,
        1.0,
        1.0
    );
}

struct Color
{
    float4 clearColor;
};

fragment float4 PSMain(constant Color& color)
{
    return color.clearColor;
}

/*
 * ClearScreen.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer Color : register(b1)
{
    float4 clearColor;
}

float4 VSMain(uint id : SV_VertexID) : SV_Position
{
	return float4(
        id == 1 ? 3.0 : -1.0,
        id == 2 ? -3.0 : 1.0,
        1.0,
        1.0
    );
}

float4 PSMain() : SV_Target0
{
    return clearColor;
}

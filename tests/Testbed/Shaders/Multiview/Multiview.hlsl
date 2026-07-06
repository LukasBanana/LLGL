/*
 * Multiview.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 *
 * Testbed multiview (single-pass layered) shader for Direct3D 12 view instancing. Renders a full-screen
 * triangle whose color depends on SV_ViewID, so each view writes a distinct color into its own array layer.
 * Requires Shader Model 6.1 (DXC).
 */

struct VSOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

VSOutput VSMain(uint id : SV_VertexID, uint viewID : SV_ViewID)
{
    VSOutput output;
    output.position = float4(
        id == 1 ?  3.0 : -1.0,
        id == 2 ? -3.0 :  1.0,
        1.0,
        1.0
    );
    // View 0 -> red, view 1 -> green: distinct per-view colors to verify view -> layer routing.
    output.color = (viewID == 0 ? float3(1.0, 0.0, 0.0) : float3(0.0, 1.0, 0.0));
    return output;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    return float4(input.color, 1.0);
}

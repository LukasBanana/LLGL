// Example.hlsl - HelloOpenXR shader for Direct3D 11 and Direct3D 12.

cbuffer Globals : register(b0)
{
    float4x4 viewProj;
};

struct VSInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

VSOutput VS(VSInput input)
{
    VSOutput output;
    output.position = mul(viewProj, float4(input.position, 1.0));
    output.color    = input.color;
    return output;
}

float4 PS(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}

// Single-pass stereo (multiview) vertex shader for Direct3D 12 view instancing. The PSO declares one view
// instance per eye; SV_ViewID selects this invocation's view so the matching view-projection matrix is used.
// Requires Shader Model 6.1 (DXC). Aliases register b0 with an array of per-view matrices.
cbuffer GlobalsMultiview : register(b0)
{
    float4x4 viewProjArray[2];
};

VSOutput VSMultiview(VSInput input, uint viewID : SV_ViewID)
{
    VSOutput output;
    output.position = mul(viewProjArray[viewID], float4(input.position, 1.0));
    output.color    = input.color;
    return output;
}

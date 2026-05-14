// Example.multiview.hlsl - multiview HelloOpenXR shader for D3D12.
// Targets shader model 6.1 (vs_6_1 / ps_6_1) because SV_ViewID requires SM 6.1+.

cbuffer Globals : register(b0)
{
    float4x4 viewProj[2];   // [0] = left eye, [1] = right eye
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

VSOutput VS(VSInput input, uint viewID : SV_ViewID)
{
    VSOutput output;
    output.position = mul(viewProj[viewID], float4(input.position, 1.0));
    output.color    = input.color;
    return output;
}

float4 PS(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}

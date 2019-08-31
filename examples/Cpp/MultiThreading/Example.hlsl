// HLSL shader

struct VertexIn
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

cbuffer Scene : register(b1)
{
    float4x4 wvpMatrix;
};

void VS(VertexIn inp, out VertexOut outp)
{
	outp.position   = mul(wvpMatrix, float4(inp.position, 1));
	outp.color      = float4(inp.texCoord.x, inp.texCoord.y, 1, 1);
}

float4 PS(VertexOut inp) : SV_Target
{
    return inp.color;
}


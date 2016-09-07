// TestShader.hlsl
// D3D12 Shader for LLGL
// 02/09/2016

cbuffer Settings : register(b0)
{
	float4x4 wvpMatrix;
};

RWBuffer<float4> outputBuffer;

struct VertexIn
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct VertexOut
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

VertexOut VS(VertexIn inp)
{
	VertexOut outp;
	
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
	outp.texCoord = inp.texCoord;
	
	outputBuffer[0] = outp.position;
	
	return outp;
}

float4 PS(VertexOut inp) : SV_Target
{
	return float4(0, 1, 0, 1);
}



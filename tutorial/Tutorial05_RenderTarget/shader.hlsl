// HLSL model shader


// VERTEX SHADER

cbuffer Settings : register(b0)
{
	float4x4 wvpMatrix;
};

struct InputVS
{
	float3 position : POSITION;
};

struct OutputVS
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
	outp.texCoord = inp.position.xy;
	return outp;
}


// PIXEL SHADER

Texture2D colorMap : register(t0);
SamplerState samplerState : register(s0);

float4 PS(OutputVS inp) : SV_Target
{
	return colorMap.Sample(samplerState, inp.texCoord);
}




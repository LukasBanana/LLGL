// HLSL model shader


// VERTEX SHADER

cbuffer Settings : register(b0)
{
	float4x4 wvpMatrix;
	float4 color;
};

struct InputVS
{
	float3 position : POSITION;
};

struct OutputVS
{
	float4 position : SV_Position;
};

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
	return outp;
}


// PIXEL SHADER

float4 PS() : SV_Target
{
	return color;
}




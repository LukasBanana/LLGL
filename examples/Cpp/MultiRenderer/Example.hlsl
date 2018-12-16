// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer Matrices : register(b0)
{
	float4x4 wvpMatrix;
};

struct InputVS
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct OutputVS
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

// Vertex shader main function
OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
	outp.texCoord = inp.texCoord;
	return outp;
}

Texture2D colorMap : register(t1);
SamplerState colorMapSampler : register(s2);

// Pixel shader main function
float4 PS(OutputVS inp) : SV_Target
{
	float4 color = colorMap.Sample(colorMapSampler, inp.texCoord);
	return lerp((float4)1, color, color.a);
};


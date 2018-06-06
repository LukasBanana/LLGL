// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer Settings : register(b0)
{
	float4x4	vpMatrix;
	float2		animationVector;
};

struct InputVS
{
	// Per-vertex attributes
	float3		position	: POSITION;
	float2		texCoord	: TEXCOORD;
	
	// Per-instance attributes
	float3		color		: COLOR;
	float4x4	wMatrix		: WMATRIX;
	float		arrayLayer	: ARRAYLAYER;
};

struct OutputVS
{
	float4 position	: SV_Position;
	float3 texCoord	: TEXCOORD;
	float3 color	: COLOR;	
};


// VERTEX SHADER

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	
	float2 offset = animationVector * inp.position.y;
	
	float4 coord = float4(
		inp.position.x + offset.x,
		inp.position.y,
		inp.position.z + offset.y,
		1.0
	);
	
	outp.position = mul(vpMatrix, mul(inp.wMatrix, coord));
	
	outp.texCoord = float3(inp.texCoord, inp.arrayLayer);
	outp.color = inp.color;
	
	return outp;
}


// PIXEL SHADER

Texture2DArray tex : register(t1);
SamplerState texSampler : register(s2);

float4 PS(OutputVS inp) : SV_Target
{
	float4 color = tex.Sample(texSampler, inp.texCoord);
	
	clip(color.a - 0.5);
	
	color.rgb *= inp.color;
	
	return color;
};


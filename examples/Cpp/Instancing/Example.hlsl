// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer Settings : register(b2)
{
	float4x4	vpMatrix;
	float4 		viewPos;
	float3		fogColor;
	float 		fogDensity;
	float2		animVec;
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
	float4 worldPos : WORLDPOS;
	float3 texCoord	: TEXCOORD;
	float3 color	: COLOR;
};


// VERTEX SHADER

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	
	float2 offset = animVec * inp.position.y;
	
	float4 coord = float4(
		inp.position.x + offset.x,
		inp.position.y,
		inp.position.z + offset.y,
		1.0
	);
	
	outp.worldPos = mul(inp.wMatrix, coord);
	outp.position = mul(vpMatrix, outp.worldPos);
	
	outp.texCoord = float3(inp.texCoord, inp.arrayLayer);
	outp.color    = inp.color;
	
	return outp;
}


// PIXEL SHADER

Texture2DArray tex : register(t3);
SamplerState texSampler : register(s4);

float4 PS(OutputVS inp) : SV_Target
{
	// Sample albed texture
	float4 color = tex.Sample(texSampler, inp.texCoord);
	
	// Apply alpha clipping
	clip(color.a - 0.5);
	
    // Compute fog density
    float viewDist = distance(viewPos, inp.worldPos);
    float fog = viewDist*fogDensity;
    fog = 1.0 - 1.0/exp(fog*fog);

    // Interpolate between albedo and fog color
    color.rgb = lerp(color.rgb * inp.color, fogColor, fog);
	
	return color;
};


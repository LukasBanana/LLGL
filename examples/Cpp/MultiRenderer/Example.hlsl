// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer Matrices : register(b1)
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
};

struct InputVS
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct OutputVS
{
	float4 position : SV_Position;
	float3 normal   : NORMAL;
	float2 texCoord : TEXCOORD;
};

// Vertex shader main function
OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
	outp.normal   = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
	outp.texCoord = inp.texCoord;
	return outp;
}

Texture2D colorMap : register(t2);
SamplerState colorMapSampler : register(s3);

// Pixel shader main function
float4 PS(OutputVS inp) : SV_Target
{
	float4 color = colorMap.Sample(colorMapSampler, inp.texCoord);

	// Sanitize texture sample
	color = lerp((float4)1, color, color.a);

	// Apply lambert factor for simple shading
	const float3 lightVec = float3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(inp.normal));
	color.rgb *= lerp(0.2, 1.0, NdotL);

	return color;
};


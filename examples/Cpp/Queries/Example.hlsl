// HLSL model shader


// VERTEX SHADER

cbuffer Settings : register(b0)
{
	float4x4 wvpMatrix;
    float4x4 wMatrix;
	float4 color;
};

struct InputVS
{
	float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct OutputVS
{
	float4 position : SV_Position;
    float3 normal   : NORMAL;
};

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
    outp.normal   = mul(wMatrix, float4(inp.normal, 0)).xyz;
	return outp;
}


// PIXEL SHADER

float4 PS(OutputVS inp) : SV_Target
{
    float3 lightDir = float3(0, 0, -1);
    float NdotL = dot(lightDir, normalize(inp.normal));
    float intensity = max(0.2, NdotL);
	return color * float4((float3)intensity, 1);
}




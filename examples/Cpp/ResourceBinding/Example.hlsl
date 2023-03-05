// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer Scene : register(b0)
{
	float4x4 vpMatrix;
}

cbuffer Model : register(b1)
{
	float3   lightVec;
	uint     instance;
}

struct VSInput
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct VSOutput
{
	float4 position : SV_Position;
	float4 worldPos : WORLDPOS;
	float3 normal   : NORMAL;
	float2 texCoord : TEXCOORD;
};


// VERTEX SHADER

struct Transform
{
	float4x4 wMatrix;
};

StructuredBuffer<Transform> transforms : register(t1);

void VSMain(VSInput inp, out VSOutput outp)
{
	Transform transform = transforms[instance];
	outp.worldPos = mul(transform.wMatrix, float4(inp.position, 1));
	outp.position = mul(vpMatrix, outp.worldPos);
	outp.normal   = mul(transform.wMatrix, float4(inp.normal, 0)).xyz;
	outp.texCoord = inp.texCoord;
}


// PIXEL SHADER

Texture2D colorMap : register(t3);
SamplerState colorMapSampler : register(s2);

float4 PSMain(VSOutput inp) : SV_Target
{
	// Sample color map
	float4 color = colorMap.Sample(colorMapSampler, inp.texCoord);

	// Compute lighting
	float3 normal = normalize(inp.normal);
	float NdotL = max(0.2, dot(normal, lightVec));
	
	return float4(color.rgb * NdotL, color.a);
};


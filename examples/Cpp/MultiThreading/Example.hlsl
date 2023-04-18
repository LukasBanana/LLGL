// HLSL shader

struct VertexIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
};

cbuffer Scene : register(b1)
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
};

void VS(VertexIn inp, out VertexOut outp)
{
	outp.position   = mul(wvpMatrix, float4(inp.position, 1));
    outp.normal     = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
	outp.color      = float4(inp.texCoord.x, inp.texCoord.y, 1, 1);
}

float4 PS(VertexOut inp) : SV_Target
{
    float4 color = inp.color;

	// Apply lambert factor for simple shading
	const float3 lightVec = float3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(inp.normal));
	color.rgb *= lerp(0.2, 1.0, NdotL);

    return color;
}


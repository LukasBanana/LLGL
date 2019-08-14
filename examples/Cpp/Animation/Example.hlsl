// HLSL model shader

cbuffer Settings : register(b1)
{
    float4x4    wMatrix;
    float4x4    vpMatrix;
    float4      lightDir;
    float4      diffuse;
};


// VERTEX SHADER SCENE

struct VIn
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VOut
{
	float4 position : SV_Position;
	float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

void VS(VIn inp, out VOut outp)
{
	outp.position	= mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
	outp.normal		= mul(wMatrix, float4(inp.normal, 0));
    outp.texCoord   = inp.texCoord;
}


// PIXEL SHADER SCENE

Texture2D colorMap : register(t2);
SamplerState linearSampler : register(s3);

float4 PS(VOut inp) : SV_Target
{
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));

    // Sample texture
    float4 color = colorMap.Sample(linearSampler, inp.texCoord);

    // Set final output color
    return lerp((float4)1, color, diffuse.a) * float4(diffuse.rgb * NdotL, 1.0);
}




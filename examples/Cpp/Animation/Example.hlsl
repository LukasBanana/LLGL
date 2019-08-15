// HLSL model shader

cbuffer Settings : register(b1)
{
    float4x4    wMatrix;
    float4x4    vpMatrix;
    float3      lightDir;
    float       shininess;
    float4      viewPos;
    float4      albedo;
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
    float4 worldPos : WORLDPOS;
	float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

void VS(VIn inp, out VOut outp)
{
    outp.worldPos   = mul(wMatrix, float4(inp.position, 1));
	outp.position	= mul(vpMatrix, outp.worldPos);
	outp.normal		= mul(wMatrix, float4(inp.normal, 0));
    outp.texCoord   = inp.texCoord;
}


// PIXEL SHADER SCENE

Texture2D colorMap : register(t2);
SamplerState linearSampler : register(s3);

float4 PS(VOut inp) : SV_Target
{
    // Diffuse lighting
    float3  lightVec    = -lightDir.xyz;
    float3  normal      = normalize(inp.normal.xyz);
    float   NdotL       = lerp(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    float3  diffuse     = albedo.rgb * NdotL;

    // Specular lighting
    float3  viewDir     = normalize(viewPos.xyz - inp.worldPos.xyz);
    float3  halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    float3  specular    = (float3)pow(max(0.0, NdotH), shininess);

    // Sample texture
    float4 color = colorMap.Sample(linearSampler, inp.texCoord);

    // Set final output color
    return lerp((float4)1, color, albedo.a) * float4(diffuse + specular, 1.0);
}




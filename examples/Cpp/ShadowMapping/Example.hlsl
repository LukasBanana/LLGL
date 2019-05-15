// HLSL model shader

cbuffer Settings : register(b1)
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    float4 lightDir;
    float4 diffuse;
};


// VERTEX SHADER SHADOW-MAP

float4 VShadowMap(float3 position : POSITION) : SV_Position
{
	return mul(vpShadowMatrix, mul(wMatrix, float4(position, 1)));
}


// VERTEX SHADER SCENE

struct InputVScene
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct OutputVScene
{
	float4 position : SV_Position;
    float4 worldPos : WORLDPOS;
	float4 normal : NORMAL;
};

OutputVScene VScene(InputVScene inp)
{
	OutputVScene outp;
    outp.worldPos   = mul(wMatrix, float4(inp.position, 1));
	outp.position	= mul(vpMatrix, outp.worldPos);
	outp.normal		= mul(wMatrix, float4(inp.normal, 0));
	return outp;
}


// PIXEL SHADER SCENE

Texture2D shadowMap : register(t2);
SamplerComparisonState shadowMapSampler : register(s3);

float4 PScene(OutputVScene inp) : SV_Target
{
    // Project world position into shadow-map space
    float4 shadowPos = mul(vpShadowMatrix, inp.worldPos);
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    // Sample shadow map
    float shadow = shadowMap.SampleCmp(shadowMapSampler, shadowPos.xy, shadowPos.z);
    
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));
    
    // Set final output color
    shadow = lerp(0.2, 1.0, shadow);
    return float4(diffuse.rgb * (NdotL * shadow), 1.0);
}




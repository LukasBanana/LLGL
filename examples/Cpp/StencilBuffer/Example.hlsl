// HLSL model shader

cbuffer SceneView : register(b1)
{
    float4x4    wMatrix;
    float4x4    vpMatrix;
    float3      lightDir;
    float       reflectionFading;
    float4      diffuse;
};


// VERTEX SHADER STENCIL-BUFFER

float4 VSStencil(float3 position : POSITION) : SV_Position
{
	return mul(vpMatrix, mul(wMatrix, float4(position, 1)));
}


// VERTEX SHADER SCENE

struct InputVScene
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct OutputVScene
{
	float4 position : SV_Position;
	float3 normal   : NORMAL;
};

OutputVScene VSScene(InputVScene inp)
{
	OutputVScene outp;
	outp.position	= mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
	outp.normal		= mul(wMatrix, float4(inp.normal, 0)).xyz;
	return outp;
}


// PIXEL SHADER SCENE

float4 ComputeLighting(float4 albedo, float3 normal)
{
    normal = normalize(normal);
    float NdotL = max(0.2, dot(normal, -lightDir));
    return float4(albedo.rgb * NdotL, albedo.a);
}

float4 PSScene(OutputVScene inp) : SV_Target
{
    return ComputeLighting(diffuse, inp.normal);
}

float4 PSReflection(OutputVScene inp) : SV_Target
{
    float4 floorColor = ComputeLighting((float4)1, float3(0, 1, 0));
    float4 meshColor = ComputeLighting(diffuse, inp.normal);
    return lerp(meshColor, floorColor, reflectionFading);
}




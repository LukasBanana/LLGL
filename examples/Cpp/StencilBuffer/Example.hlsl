// HLSL model shader

cbuffer Settings : register(b1)
{
    float4x4    wMatrix;
    float4x4    vpMatrix;
    float4      lightDir;
    float4      diffuse;
};


// VERTEX SHADER STENCIL-BUFFER

float4 VStencil(float3 position : POSITION) : SV_Position
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
	float4 normal   : NORMAL;
};

OutputVScene VScene(InputVScene inp)
{
	OutputVScene outp;
	outp.position	= mul(vpMatrix, mul(wMatrix, float4(inp.position, 1)));
	outp.normal		= mul(wMatrix, float4(inp.normal, 0));
	return outp;
}


// PIXEL SHADER SCENE

float4 PScene(OutputVScene inp) : SV_Target
{
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));

    // Set final output color
    return float4(diffuse.rgb * NdotL, 1.0);
}




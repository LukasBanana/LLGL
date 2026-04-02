// Shader Model 3 HLSL texturing shader for D3D9 backend


// VERTEX SHADER

struct Scene
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float3   lightVec;
};

uniform Scene scene;

struct InputVS
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct OutputVS
{
    float4 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

OutputVS VS(InputVS inp)
{
    OutputVS outp;
    outp.position = mul(scene.wvpMatrix, float4(inp.position, 1));
    outp.normal   = normalize(mul(scene.wMatrix, float4(inp.normal, 0)).xyz);
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

sampler2D colorMap : register(s2);

float4 PS(OutputVS inp) : SV_Target
{
    float4 color = tex2D(colorMap, inp.texCoord);//.bgra;

	// Apply lambert factor for simple shading
	float NdotL = dot(scene.lightVec, normalize(inp.normal));
	color.rgb *= lerp(0.2, 1.0, NdotL);

    return color;
}




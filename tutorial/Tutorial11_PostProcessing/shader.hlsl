// HLSL shader


// SCENE SHADERS

cbuffer SceneSettings : register(b0)
{
	float4x4	wvpMatrix;
	float4x4	wMatrix;
	float4		glossiness;
};

struct InputVScene
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct OutputVScene
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
};

OutputVScene VScene(InputVScene inp)
{
	OutputVScene outp;
	outp.position = mul(wvpMatrix, float4(inp.position, 1));
	outp.normal = mul(wMatrix, float4(inp.normal, 0)).xyz;
	return outp;
}

struct OutputPScene
{
	float4 color : SV_Target0;
	float4 gloss : SV_Target1;
};

OutputPScene PScene(InputVScene inp)
{
	OutputPScene outp;
	
	// Write simple lighting into 1st render target
	float3 lightDir = float3(0, 0, -1);
	float3 normal = normalize(inp.normal);
	
	float NdotL = max(0.4, dot(lightDir, normal));
	outp.color = float4((float3)NdotL, 1);
	
	// Write glossiness out into 2nd render target
	outp.gloss = glossiness;
	
	return outp;
}


// POST-PROCESSING SHADERS

float4 GetFullscreenTriangleVertex(uint id)
{
	return float4(
		(id == 2 ?  3.0 : -1.0),
		(id == 0 ? -3.0 :  1.0),
		1.0,
		1.0
	);
}

cbuffer BlurSettings : register(b1)
{
	float2 blurShift;
};

struct OutputVPP
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

OutputVPP VPP(uint id : SV_VertexID)
{
	OutputVPP outp;
	outp.position = GetFullscreenTriangleVertex(id);
	outp.texCoord = outp.position.xy * 0.5 + 0.5;
	return outp;
}

Texture2D colorMap : register(t0);
Texture2D glossMap : register(t1);

SamplerState colorMapSampler : register(s0);
SamplerState glossMapSampler : register(s1);

#define GAUSSIAN_KERNEL_1 0.00598
#define GAUSSIAN_KERNEL_2 0.060626
#define GAUSSIAN_KERNEL_3 0.241843
#define GAUSSIAN_KERNEL_4 0.383103

float4 PBlur(OutputVPP inp) : SV_Target
{
	// Accumulate the samples with the gaussian kernel (sigma = 1.0, size = 7)
	// see http://dev.theomader.com/gaussian-kernel-calculator/
	float4 c = (float4)0;
	
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift*3) * GAUSSIAN_KERNEL_1;
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift*2) * GAUSSIAN_KERNEL_2;
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift  ) * GAUSSIAN_KERNEL_3;
	c += glossMap.Sample(glossMapSampler, inp.texCoord              ) * GAUSSIAN_KERNEL_4;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift  ) * GAUSSIAN_KERNEL_3;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift*2) * GAUSSIAN_KERNEL_2;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift*3) * GAUSSIAN_KERNEL_1;
	
	return c;
}

float4 PFinal(OutputVPP inp) : SV_Target
{
	// Show final result with color and gloss map
	return
		colorMap.Sample(colorMapSampler, inp.texCoord) +
		glossMap.Sample(glossMapSampler, inp.texCoord);
};





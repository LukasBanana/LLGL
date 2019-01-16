// HLSL shader


// SCENE SHADERS

cbuffer SceneSettings : register(b1)
{
	float4x4	wvpMatrix;
	float4x4	wMatrix;
	float4		diffuse;
	float4		glossiness;
	float		intensity;
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
	outp.position	= mul(wvpMatrix, float4(inp.position, 1));
	outp.normal		= mul(wMatrix, float4(inp.normal, 0)).xyz;
	return outp;
}

struct OutputPScene
{
	float4 color : SV_Target0;
	float4 gloss : SV_Target1;
};

OutputPScene PScene(OutputVScene inp)
{
	OutputPScene outp;
	
	// Write simple lighting into 1st render target
	float3 lightDir = float3(0, 0, -1);
	float3 normal = normalize(inp.normal);
	
	float NdotL = max(0.4, dot(lightDir, normal));
	outp.color = diffuse * float4((float3)NdotL, 1);
	
	// Write glossiness into 2nd render target
	outp.gloss = glossiness;
	
	return outp;
}


// POST-PROCESSING SHADERS

/*
This function generates the coordinates for a fullscreen triangle with its vertex IDs:

(-1,+3)
   *
   | \
   |   \
   |     \
   |       \
(-1,+1)    (+1,+1)
   *----------*\
   |          |  \
   |  screen  |    \
   |          |      \
   *----------*--------*
(-1,-1)    (+1,-1)  (+3,-1)
*/
float4 GetFullscreenTriangleVertex(uint id)
{
	return float4(
		(id == 2 ? 3.0 : -1.0),
		(id == 0 ? 3.0 : -1.0),
		1.0,
		1.0
	);
}

cbuffer BlurSettings : register(b2)
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
	
	// Generate coorindate for fullscreen triangle
	outp.position = GetFullscreenTriangleVertex(id);
	
	// Get texture-coordinate from vertex position
	outp.texCoord = outp.position.xy * float2(0.5, -0.5) + 0.5;
	
	return outp;
}

Texture2D colorMap : register(t3);
Texture2D glossMap : register(t4);

SamplerState colorMapSampler : register(s5);
SamplerState glossMapSampler : register(s6);

// Static values for the 1-dimensional gaussian kernel (sigma = 2.2, size = 11)
// see http://dev.theomader.com/gaussian-kernel-calculator/
#define GAUSSIAN_KERNEL_1 0.014374
#define GAUSSIAN_KERNEL_2 0.035855
#define GAUSSIAN_KERNEL_3 0.072994
#define GAUSSIAN_KERNEL_4 0.121281
#define GAUSSIAN_KERNEL_5 0.164472
#define GAUSSIAN_KERNEL_6 0.182049

float4 PBlur(OutputVPP inp) : SV_Target
{
	// Accumulate the samples with the gaussian kernel
	float4 c = (float4)0;
	
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift*5) * GAUSSIAN_KERNEL_1;
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift*4) * GAUSSIAN_KERNEL_2;
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift*3) * GAUSSIAN_KERNEL_3;
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift*2) * GAUSSIAN_KERNEL_4;
	c += glossMap.Sample(glossMapSampler, inp.texCoord - blurShift  ) * GAUSSIAN_KERNEL_5;
	c += glossMap.Sample(glossMapSampler, inp.texCoord              ) * GAUSSIAN_KERNEL_6;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift  ) * GAUSSIAN_KERNEL_5;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift*2) * GAUSSIAN_KERNEL_4;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift*3) * GAUSSIAN_KERNEL_3;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift*4) * GAUSSIAN_KERNEL_2;
	c += glossMap.Sample(glossMapSampler, inp.texCoord + blurShift*5) * GAUSSIAN_KERNEL_1;
	
	return c;
}

float4 PFinal(OutputVPP inp) : SV_Target
{
	// Show final result with color and gloss map
	return
		colorMap.Sample(colorMapSampler, inp.texCoord) +
		glossMap.Sample(glossMapSampler, inp.texCoord) * intensity;
};





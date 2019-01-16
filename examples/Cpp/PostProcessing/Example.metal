// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;


// SCENE SHADERS

struct SceneSettings
{
    float4x4    wvpMatrix;
    float4x4    wMatrix;
    float4      diffuse;
    float4      glossiness;
    float       intensity;
};

struct InputVScene
{
	float3 position [[attribute(0)]];
	float3 normal   [[attribute(1)]];
};

struct OutputVScene
{
	float4 position [[position]];
	float3 normal;
};

vertex OutputVScene VScene(
    InputVScene             inp      [[stage_in]],
    constant SceneSettings& settings [[buffer(1)]])
{
	OutputVScene outp;
	outp.position	= settings.wvpMatrix * float4(inp.position, 1);
	outp.normal		= (settings.wMatrix * float4(inp.normal, 0)).xyz;
	return outp;
}

struct OutputPScene
{
	float4 color [[color(0)]];
	float4 gloss [[color(1)]];
};

fragment OutputPScene PScene(
    OutputVScene            inp      [[stage_in]],
    constant SceneSettings& settings [[buffer(1)]])
{
	OutputPScene outp;
	
	// Write simple lighting into 1st render target
	float3 lightDir = float3(0, 0, -1);
	float3 normal = normalize(inp.normal);
	
	float NdotL = max(0.4, dot(lightDir, normal));
	outp.color = settings.diffuse * float4((float3)NdotL, 1);
	
	// Write glossiness into 2nd render target
	outp.gloss = settings.glossiness;
	
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

struct BlurSettings
{
	float2 blurShift;
};

struct OutputVPP
{
	float4 position [[position]];
	float2 texCoord;
};

vertex OutputVPP VPP(uint id [[vertex_id]])
{
	OutputVPP outp;
	
	// Generate coorindate for fullscreen triangle
	outp.position = GetFullscreenTriangleVertex(id);
	
	// Get texture-coordinate from vertex position
	outp.texCoord = outp.position.xy * float2(0.5, -0.5) + 0.5;
	
	return outp;
}

// Static values for the 1-dimensional gaussian kernel (sigma = 2.2, size = 11)
// see http://dev.theomader.com/gaussian-kernel-calculator/
#define GAUSSIAN_KERNEL_1 0.014374
#define GAUSSIAN_KERNEL_2 0.035855
#define GAUSSIAN_KERNEL_3 0.072994
#define GAUSSIAN_KERNEL_4 0.121281
#define GAUSSIAN_KERNEL_5 0.164472
#define GAUSSIAN_KERNEL_6 0.182049

fragment float4 PBlur(
    OutputVPP               inp             [[stage_in]],
    constant BlurSettings&  settings        [[buffer(2)]],
    texture2d<float>        glossMap        [[texture(4)]],
    sampler                 glossMapSampler [[sampler(6)]])
{
	// Accumulate the samples with the gaussian kernel
	float4 c = (float4)0;
	
	c += glossMap.sample(glossMapSampler, inp.texCoord - settings.blurShift*5) * GAUSSIAN_KERNEL_1;
	c += glossMap.sample(glossMapSampler, inp.texCoord - settings.blurShift*4) * GAUSSIAN_KERNEL_2;
	c += glossMap.sample(glossMapSampler, inp.texCoord - settings.blurShift*3) * GAUSSIAN_KERNEL_3;
	c += glossMap.sample(glossMapSampler, inp.texCoord - settings.blurShift*2) * GAUSSIAN_KERNEL_4;
	c += glossMap.sample(glossMapSampler, inp.texCoord - settings.blurShift  ) * GAUSSIAN_KERNEL_5;
	c += glossMap.sample(glossMapSampler, inp.texCoord                       ) * GAUSSIAN_KERNEL_6;
	c += glossMap.sample(glossMapSampler, inp.texCoord + settings.blurShift  ) * GAUSSIAN_KERNEL_5;
	c += glossMap.sample(glossMapSampler, inp.texCoord + settings.blurShift*2) * GAUSSIAN_KERNEL_4;
	c += glossMap.sample(glossMapSampler, inp.texCoord + settings.blurShift*3) * GAUSSIAN_KERNEL_3;
	c += glossMap.sample(glossMapSampler, inp.texCoord + settings.blurShift*4) * GAUSSIAN_KERNEL_2;
	c += glossMap.sample(glossMapSampler, inp.texCoord + settings.blurShift*5) * GAUSSIAN_KERNEL_1;
	
	return c;
}

fragment float4 PFinal(
    OutputVPP               inp             [[stage_in]],
    constant SceneSettings& settings        [[buffer(1)]],
    texture2d<float>        colorMap        [[texture(3)]],
    texture2d<float>        glossMap        [[texture(4)]],
    sampler                 colorMapSampler [[sampler(5)]],
    sampler                 glossMapSampler [[sampler(6)]])
{
	// Show final result with color and gloss map
	return
		colorMap.sample(colorMapSampler, inp.texCoord) +
		glossMap.sample(glossMapSampler, inp.texCoord) * settings.intensity;
};





// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Matrices
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
};

struct InputVS
{
	float3 position [[attribute(0)]];
	float3 normal   [[attribute(1)]];
	float2 texCoord [[attribute(2)]];
};

struct OutputVS
{
	float4 position [[position]];
	float3 normal;
	float2 texCoord;
};

// Vertex shader main function
vertex OutputVS VS(
    InputVS            inp      [[stage_in]],
    constant Matrices& matrices [[buffer(1)]])
{
	OutputVS outp;
	outp.position = matrices.wvpMatrix * float4(inp.position, 1);
	outp.normal   = normalize(matrices.wMatrix * float4(inp.normal, 0)).xyz;
	outp.texCoord = inp.texCoord;
	return outp;
}

// Pixel shader main function
fragment float4 PS(
    OutputVS         inp             [[stage_in]],
    texture2d<float> colorMap        [[texture(2)]],
    sampler          colorMapSampler [[sampler(3)]])
{
	float4 color = colorMap.sample(colorMapSampler, inp.texCoord);

	// Sanitize texture sample
	color = mix((float4)1, color, color.a);

	// Apply lambert factor for simple shading
	const float3 lightVec = float3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(inp.normal));
	color.rgb *= mix(0.2, 1.0, NdotL);

	return color;
};


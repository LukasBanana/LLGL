// Metal shader 2.0

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct InputVS
{
	float2 position [[attribute(0)]];
	float3 color    [[attribute(1)]];
};

struct OutputVS
{
	float4  position [[position]];
	float3  color;
    uint    viewport [[viewport_array_index]];
};

// Vertex shader main function
vertex OutputVS VS(
    InputVS inp [[stage_in]],
    uint instID [[instance_id]])
{
	OutputVS outp;
	outp.position   = float4(inp.position * mix(1.0, -1.0, (float)instID), 0, 1);
	outp.color      = inp.color;
    outp.viewport   = instID;
	return outp;
}

// Pixel shader main function
fragment float4 PS(OutputVS inp [[stage_in]])
{
	return float4(inp.color, 1);
};


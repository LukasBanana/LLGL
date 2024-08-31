// Metal shader for queries example

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// VERTEX SHADER

struct InputVS
{
	float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
};

struct OutputVS
{
	float4 position [[position]];
    float3 normal;
};

struct Scene
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4   color;
};

vertex OutputVS VS(
    InputVS         inp   [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
	OutputVS outp;
	outp.position = scene.wvpMatrix * float4(inp.position, 1);
    outp.normal   = (scene.wMatrix * float4(inp.normal, 0)).xyz;
	return outp;
}


// PIXEL SHADER

fragment float4 PS(
    OutputVS        inp   [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    float3 lightDir = float3(0, 0, -1);
    float NdotL = dot(lightDir, normalize(inp.normal));
    float intensity = max(0.2, NdotL);
	return scene.color * float4((float3)intensity, 1);
}




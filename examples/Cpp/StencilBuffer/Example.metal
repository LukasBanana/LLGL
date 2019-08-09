// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Settings
{
    float4x4    wMatrix;
    float4x4    vpMatrix;
    float4      lightDir;
    float4      diffuse;
};


// VERTEX SHADER STENCIL-BUFFER

struct VStencilIn
{
    float3 position [[attribute(0)]];
};

struct VStencilOut
{
    float4 position [[position]];
};

vertex VStencilOut VStencil(
    VStencilIn         inp      [[stage_in]],
    constant Settings& settings [[buffer(1)]])
{
    VStencilOut outp;
	outp.position = settings.vpMatrix * (settings.wMatrix * float4(inp.position, 1));
    return outp;
}


// VERTEX SHADER SCENE

struct VSceneIn
{
	float3 position [[attribute(0)]];
	float3 normal   [[attribute(1)]];
};

struct VSceneOut
{
	float4 position [[position]];
	float4 normal;
};

vertex VSceneOut VScene(
    VSceneIn           inp      [[stage_in]],
    constant Settings& settings [[buffer(1)]])
{
	VSceneOut outp;
	outp.position	= settings.vpMatrix * (settings.wMatrix * float4(inp.position, 1));
	outp.normal		= settings.wMatrix * float4(inp.normal, 0);
	return outp;
}


// PIXEL SHADER SCENE

fragment float4 PScene(
    VSceneOut          inp      [[stage_in]],
    constant Settings& settings [[buffer(1)]])
{
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(normal, -settings.lightDir.xyz));

    // Set final output color
    return float4(settings.diffuse.rgb * NdotL, 1.0);
}




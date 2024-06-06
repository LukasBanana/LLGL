// Metal vertex

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 normal;
    float4 color;
};

struct Scene
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
};

vertex VertexOut VS(
    VertexIn        inp     [[stage_in]],
    constant Scene& scene   [[buffer(1)]])
{
    VertexOut outp;
	outp.position   = scene.wvpMatrix * float4(inp.position, 1);
    outp.normal     = normalize((scene.wMatrix * float4(inp.normal, 0)).xyz);
	outp.color      = float4(inp.texCoord.x, inp.texCoord.y, 1, 1);
    return outp;
}

fragment float4 PS(VertexOut inp [[stage_in]])
{
    float4 color = inp.color;

    // Apply lambert factor for simple shading
    const float3 lightVec = float3(0, 0, -1);
    float NdotL = dot(lightVec, normalize(inp.normal));
    color.rgb *= mix(0.2, 1.0, NdotL);

    return color;
}


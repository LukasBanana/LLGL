// Metal vertex

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexIn
{
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct VertexOut
{
    float4 position [[position]];
    float4 color;
};

struct Scene
{
    float4x4 wvpMatrix;
};

vertex VertexOut VS(
    VertexIn        inp     [[stage_in]],
    constant Scene& scene   [[buffer(1)]])
{
    VertexOut outp;
	outp.position   = scene.wvpMatrix * float4(inp.position, 1);
	outp.color      = float4(inp.texCoord.x, inp.texCoord.y, 1, 1);
    return outp;
}

fragment float4 PS(VertexOut inp [[stage_in]])
{
    return inp.color;
}


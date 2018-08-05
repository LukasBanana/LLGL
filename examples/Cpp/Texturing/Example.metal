
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
}
VertexIn;

typedef struct
{
    float4 position [[position]];
    float2 texCoord;
}
VertexOut;

vertex VertexOut VS(VertexIn inp [[stage_in]])
{
    VertexOut outp;

    outp.position = float4(inp.position, 0.0, 1.0);
    outp.texCoord = inp.texCoord;

    return outp;
}

fragment float4 PS(
    VertexOut           inp [[stage_in]],
    texture2d<float>    tex [[texture(1)]],
    sampler             smpl [[sampler(0)]])
{
    return tex.sample(smpl, inp.texCoord);
}

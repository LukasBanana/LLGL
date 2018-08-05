
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float3 color    [[attribute(2)]];
}
VertexIn;

typedef struct
{
    float4 position [[position]];
    float2 texCoord;
    float4 color;
}
VertexOut;

vertex VertexOut VMain(VertexIn inp [[stage_in]])
{
    VertexOut outp;

    outp.position = float4(inp.position, 0.0, 1.0);
    outp.texCoord = inp.texCoord * 3.0 - 1.0;
    outp.color    = float4(inp.color, 1.0);

    return outp;
}

fragment float4 FMain(
    VertexOut           inp [[stage_in]],
    texture2d<float>    tex [[texture(0)]],
    sampler             smpl [[sampler(0)]])
{
    constexpr sampler smpl2(mag_filter::linear, min_filter::linear);
    #if 0
    return inp.color * tex.sample(smpl, inp.texCoord);
    #else
    return tex.sample(smpl, inp.texCoord);
    #endif
}

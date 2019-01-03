
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float2 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
}
VertexIn;

typedef struct
{
    float4 position [[position]];
    float4 color;
}
VertexOut;

vertex VertexOut VS(VertexIn inp [[stage_in]])
{
    VertexOut outp;

    outp.position = float4(inp.position, 0.0, 1.0);
    outp.color    = inp.color;

    return outp;
}

fragment float4 PS(VertexOut inp [[stage_in]])
{
    return inp.color;
}

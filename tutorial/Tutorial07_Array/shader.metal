
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float2 position       [[attribute(0)]];
    float3 color          [[attribute(1)]];
    float3 instanceColor  [[attribute(2)]];
    float2 instanceOffset [[attribute(3)]];
    float  instanceScale  [[attribute(4)]];
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

    outp.position = float4(inp.position * inp.instanceScale + inp.instanceOffset, 0.0, 1.0);
    outp.color    = float4(inp.color * inp.instanceColor, 1.0);

    return outp;
}

fragment float4 PS(VertexOut inp [[stage_in]])
{
    return inp.color;
}

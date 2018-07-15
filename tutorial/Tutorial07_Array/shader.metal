
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float2 position [[attribute(0)]];
    float3 color    [[attribute(1)]];
}
VertexIn;

typedef struct
{
    float3 instanceColor;
    float2 instanceOffset;
    float  instanceScale;
}
InstanceIn;

typedef struct
{
    float4 position [[position]];
    float4 color;
}
VertexOut;

vertex VertexOut VS(
    VertexIn             inp      [[stage_in]],
    constant InstanceIn* instance [[buffer(2)]],
    uint                 instID   [[instance_id]])
{
    VertexOut outp;

    outp.position = float4(inp.position * instance[instID].instanceScale + instance[instID].instanceOffset, 0.0, 1.0);
    outp.color    = float4(inp.color * instance[instID].instanceColor, 1.0);

    return outp;
}

fragment float4 PS(VertexOut inp [[stage_in]])
{
    return inp.color;
}

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VPP_out
{
    float2 out_var_TEXCOORD [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex VPP_out VPP(uint gl_VertexIndex [[vertex_id]])
{
    VPP_out out = {};
    float4 _30 = float4((gl_VertexIndex == 2u) ? 3.0 : (-1.0), (gl_VertexIndex == 0u) ? 3.0 : (-1.0), 1.0, 1.0);
    out.gl_Position = _30;
    out.out_var_TEXCOORD = (_30.xy * float2(0.5, -0.5)) + float2(0.5);
    return out;
}


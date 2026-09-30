#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VS_out
{
    float3 out_var_COLOR [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float2 in_var_POSITION [[attribute(0)]];
    float3 in_var_COLOR [[attribute(1)]];
};

vertex VS_out VS(VS_in in [[stage_in]])
{
    VS_out out = {};
    out.gl_Position = float4(in.in_var_POSITION, 0.0, 1.0);
    out.out_var_COLOR = in.in_var_COLOR;
    return out;
}


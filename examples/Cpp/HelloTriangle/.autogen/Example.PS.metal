#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_COLOR [[user(locn0)]];
};

fragment PS_out PS(PS_in in [[stage_in]])
{
    PS_out out = {};
    out.out_var_SV_Target = float4(in.in_var_COLOR, 1.0);
    return out;
}


#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 color;
    float3 lightDir;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_NORMAL [[user(locn0)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    PS_out out = {};
    float _29 = precise::max(0.20000000298023223876953125, dot(Settings.lightDir, fast::normalize(in.in_var_NORMAL)));
    out.out_var_SV_Target = Settings.color * float4(_29, _29, _29, 1.0);
    return out;
}


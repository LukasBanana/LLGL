#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4 lightDir;
    float4 diffuse;
};

struct PScene_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PScene_in
{
    float4 in_var_NORMAL [[user(locn0)]];
};

fragment PScene_out PScene(PScene_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    PScene_out out = {};
    out.out_var_SV_Target = float4(Settings.diffuse.xyz * precise::max(0.20000000298023223876953125, dot(fast::normalize(in.in_var_NORMAL.xyz), -Settings.lightDir.xyz)), 1.0);
    return out;
}


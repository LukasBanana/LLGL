#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneSettings
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 diffuse;
    float4 glossiness;
    packed_float3 lightDir;
    float intensity;
};

struct PScene_out
{
    float4 out_var_SV_Target0 [[color(0)]];
    float4 out_var_SV_Target1 [[color(1)]];
};

struct PScene_in
{
    float3 in_var_NORMAL [[user(locn0)]];
};

fragment PScene_out PScene(PScene_in in [[stage_in]], constant type_SceneSettings& SceneSettings [[buffer(1)]])
{
    PScene_out out = {};
    float _31 = precise::max(0.4000000059604644775390625, dot(float3(SceneSettings.lightDir), fast::normalize(in.in_var_NORMAL)));
    out.out_var_SV_Target0 = SceneSettings.diffuse * float4(_31, _31, _31, 1.0);
    out.out_var_SV_Target1 = SceneSettings.glossiness;
    return out;
}


#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneView
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    packed_float3 lightDir;
    float reflectionFading;
    float4 diffuse;
};

struct PSScene_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PSScene_in
{
    float3 in_var_NORMAL [[user(locn0)]];
};

fragment PSScene_out PSScene(PSScene_in in [[stage_in]], constant type_SceneView& SceneView [[buffer(1)]])
{
    PSScene_out out = {};
    out.out_var_SV_Target = float4(SceneView.diffuse.xyz * precise::max(0.20000000298023223876953125, dot(fast::normalize(in.in_var_NORMAL), -float3(SceneView.lightDir))), SceneView.diffuse.w);
    return out;
}


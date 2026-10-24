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

struct PSReflection_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PSReflection_in
{
    float3 in_var_NORMAL [[user(locn0)]];
};

fragment PSReflection_out PSReflection(PSReflection_in in [[stage_in]], constant type_SceneView& SceneView [[buffer(1)]])
{
    PSReflection_out out = {};
    float3 _33 = -float3(SceneView.lightDir);
    out.out_var_SV_Target = mix(float4(SceneView.diffuse.xyz * precise::max(0.20000000298023223876953125, dot(fast::normalize(in.in_var_NORMAL), _33)), SceneView.diffuse.w), float4(float3(1.0) * precise::max(0.20000000298023223876953125, dot(fast::normalize(float3(0.0, 1.0, 0.0)), _33)), 1.0), float4(SceneView.reflectionFading));
    return out;
}


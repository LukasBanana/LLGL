#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Scene
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float3 lightVec;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_NORMAL [[user(locn0)]];
    float4 in_var_COLOR [[user(locn1)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_Scene& Scene [[buffer(1)]])
{
    PS_out out = {};
    float3 _32 = in.in_var_COLOR.xyz * mix(0.20000000298023223876953125, 1.0, dot(Scene.lightVec, fast::normalize(in.in_var_NORMAL)));
    out.out_var_SV_Target = float4(_32.x, _32.y, _32.z, in.in_var_COLOR.w);
    return out;
}


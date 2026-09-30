#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    packed_float3 lightDir;
    float shininess;
    float4 viewPos;
    float4 albedo;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float4 in_var_WORLDPOS [[user(locn0)]];
    float4 in_var_NORMAL [[user(locn1)]];
    float2 in_var_TEXCOORD [[user(locn2)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]], texture2d<float> colorMap [[texture(2)]], sampler linearSampler [[sampler(3)]])
{
    PS_out out = {};
    float3 _45 = -float3(Settings.lightDir);
    float3 _47 = fast::normalize(in.in_var_NORMAL.xyz);
    out.out_var_SV_Target = mix(float4(1.0), colorMap.sample(linearSampler, in.in_var_TEXCOORD), float4(Settings.albedo.w)) * float4((Settings.albedo.xyz * mix(0.20000000298023223876953125, 1.0, precise::max(0.0, dot(_47, _45)))) + float3(powr(precise::max(0.0, dot(_47, fast::normalize(fast::normalize(Settings.viewPos.xyz - in.in_var_WORLDPOS.xyz) + _45))), Settings.shininess)), 1.0);
    return out;
}


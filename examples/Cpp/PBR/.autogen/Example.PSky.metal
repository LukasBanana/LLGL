#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 cMatrix;
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float2 aspectRatio;
    float mipCount;
    float _pad0;
    float4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uint2 _pad1;
};

struct PSky_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PSky_in
{
    float4 in_var_VIEWRAY [[user(locn0)]];
};

fragment PSky_out PSky(PSky_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]], texturecube_array<float> skyBox [[texture(3)]], sampler smpl [[sampler(2)]], bool gl_FrontFacing [[front_facing]])
{
    PSky_out out = {};
    float4 _55 = float4(fast::normalize(Settings.cMatrix * float4(in.in_var_VIEWRAY.xy, gl_FrontFacing ? (-1.0) : 1.0, 0.0)).xyz, float(Settings.skyboxLayer));
    out.out_var_SV_Target = skyBox.sample(smpl, _55.xyz, uint(rint(_55.w)));
    return out;
}


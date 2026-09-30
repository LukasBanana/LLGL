#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_NORMAL [[user(locn0)]];
    float2 in_var_TEXCOORD [[user(locn1)]];
};

fragment PS_out PS(PS_in in [[stage_in]], texture2d<float> colorMap [[texture(2)]], sampler colorMapSampler [[sampler(3)]])
{
    PS_out out = {};
    float4 _34 = colorMap.sample(colorMapSampler, in.in_var_TEXCOORD);
    float4 _37 = mix(float4(1.0), _34, float4(_34.w));
    float3 _42 = _37.xyz * mix(0.20000000298023223876953125, 1.0, dot(float3(0.0, 0.0, -1.0), fast::normalize(in.in_var_NORMAL)));
    out.out_var_SV_Target = float4(_42.x, _42.y, _42.z, _37.w);
    return out;
}


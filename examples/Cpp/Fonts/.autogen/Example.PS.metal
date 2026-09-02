#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float2 in_var_TEXCOORD [[user(locn0)]];
    float4 in_var_COLOR [[user(locn1)]];
};

fragment PS_out PS(PS_in in [[stage_in]], texture2d<float> glyphTexture [[texture(0)]], sampler linearSampler [[sampler(2)]])
{
    PS_out out = {};
    out.out_var_SV_Target = float4(in.in_var_COLOR.xyz, in.in_var_COLOR.w * glyphTexture.sample(linearSampler, in.in_var_TEXCOORD).w);
    return out;
}


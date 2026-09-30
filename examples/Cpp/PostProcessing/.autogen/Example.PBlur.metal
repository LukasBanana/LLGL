#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_BlurSettings
{
    float2 blurShift;
};

struct PBlur_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PBlur_in
{
    float2 in_var_TEXCOORD [[user(locn0)]];
};

fragment PBlur_out PBlur(PBlur_in in [[stage_in]], constant type_BlurSettings& BlurSettings [[buffer(2)]], texture2d<float> glossMap [[texture(4)]], sampler glossMapSampler [[sampler(6)]])
{
    PBlur_out out = {};
    float2 _40 = BlurSettings.blurShift * 5.0;
    float2 _45 = BlurSettings.blurShift * 4.0;
    float2 _51 = BlurSettings.blurShift * 3.0;
    float2 _57 = BlurSettings.blurShift * 2.0;
    out.out_var_SV_Target = ((((((((((glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD - _40)) * 0.014374000020325183868408203125) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD - _45)) * 0.0358549989759922027587890625)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD - _51)) * 0.072994001209735870361328125)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD - _57)) * 0.1212809979915618896484375)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD - BlurSettings.blurShift)) * 0.16447199881076812744140625)) + (glossMap.sample(glossMapSampler, in.in_var_TEXCOORD) * 0.1820490062236785888671875)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD + BlurSettings.blurShift)) * 0.16447199881076812744140625)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD + _57)) * 0.1212809979915618896484375)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD + _51)) * 0.072994001209735870361328125)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD + _45)) * 0.0358549989759922027587890625)) + (glossMap.sample(glossMapSampler, (in.in_var_TEXCOORD + _40)) * 0.014374000020325183868408203125);
    return out;
}


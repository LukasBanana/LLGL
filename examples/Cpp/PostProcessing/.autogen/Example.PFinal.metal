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

struct PFinal_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PFinal_in
{
    float2 in_var_TEXCOORD [[user(locn0)]];
};

fragment PFinal_out PFinal(PFinal_in in [[stage_in]], constant type_SceneSettings& SceneSettings [[buffer(1)]], texture2d<float> colorMap [[texture(3)]], texture2d<float> glossMap [[texture(4)]], sampler colorMapSampler [[sampler(5)]], sampler glossMapSampler [[sampler(6)]])
{
    PFinal_out out = {};
    out.out_var_SV_Target = colorMap.sample(colorMapSampler, in.in_var_TEXCOORD) + (glossMap.sample(glossMapSampler, in.in_var_TEXCOORD) * SceneSettings.intensity);
    return out;
}


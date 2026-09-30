#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    packed_float3 lightDir;
    int useTexture2DMS;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_NORMAL [[user(locn0)]];
    float2 in_var_TEXCOORD [[user(locn1)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_Settings& Settings [[buffer(3)]], texture2d<float> colorMap [[texture(2)]], sampler samplerState [[sampler(1)]])
{
    PS_out out = {};
    float4 _37 = colorMap.sample(samplerState, in.in_var_TEXCOORD);
    float3 _44 = _37.xyz * mix(0.20000000298023223876953125, 1.0, dot(float3(Settings.lightDir), fast::normalize(in.in_var_NORMAL)));
    out.out_var_SV_Target = float4(_44.x, _44.y, _44.z, _37.w);
    return out;
}


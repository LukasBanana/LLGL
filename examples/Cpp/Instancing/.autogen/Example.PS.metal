#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 vpMatrix;
    float4 viewPos;
    packed_float3 fogColor;
    float fogDensity;
    float2 animVec;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float4 in_var_WORLDPOS [[user(locn0)]];
    float3 in_var_TEXCOORD [[user(locn1)]];
    float3 in_var_COLOR [[user(locn2)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_Settings& Settings [[buffer(2)]], texture2d_array<float> tex [[texture(3)]], sampler texSampler [[sampler(4)]])
{
    PS_out out = {};
    float4 _45 = tex.sample(texSampler, in.in_var_TEXCOORD.xy, uint(rint(in.in_var_TEXCOORD.z)));
    if ((_45.w - 0.5) < 0.0)
    {
        discard_fragment();
    }
    float _56 = distance(Settings.viewPos, in.in_var_WORLDPOS) * Settings.fogDensity;
    float3 _66 = mix(_45.xyz * in.in_var_COLOR, float3(Settings.fogColor), float3(1.0 - (1.0 / exp(_56 * _56))));
    out.out_var_SV_Target = float4(_66.x, _66.y, _66.z, _45.w);
    return out;
}


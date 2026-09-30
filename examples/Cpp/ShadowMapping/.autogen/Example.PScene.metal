#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template <typename T>
static inline depth2d<T> spvDepthCast(texture2d<T> t)
{
    return reinterpret_cast<thread const depth2d<T> &>(t);
}

template <typename T>
static inline depth2d_array<T> spvDepthCast(texture2d_array<T> t)
{
    return reinterpret_cast<thread const depth2d_array<T> &>(t);
}

template <typename T>
static inline depthcube<T> spvDepthCast(texturecube<T> t)
{
    return reinterpret_cast<thread const depthcube<T> &>(t);
}

template <typename T>
static inline depthcube_array<T> spvDepthCast(texturecube_array<T> t)
{
    return reinterpret_cast<thread const depthcube_array<T> &>(t);
}

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    float4 lightDir;
    float4 diffuse;
};

struct PScene_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PScene_in
{
    float4 in_var_WORLDPOS [[user(locn0)]];
    float4 in_var_NORMAL [[user(locn1)]];
};

fragment PScene_out PScene(PScene_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]], texture2d<float> shadowMap [[texture(2)]], sampler shadowMapSampler [[sampler(3)]])
{
    PScene_out out = {};
    float4 _43 = Settings.vpShadowMatrix * in.in_var_WORLDPOS;
    float4 _46 = _43 / float4(_43.w);
    out.out_var_SV_Target = float4(Settings.diffuse.xyz * (precise::max(0.20000000298023223876953125, dot(fast::normalize(in.in_var_NORMAL.xyz), -Settings.lightDir.xyz)) * mix(0.20000000298023223876953125, 1.0, spvDepthCast(shadowMap).sample_compare(shadowMapSampler, ((_46.xy * float2(0.5, -0.5)) + float2(0.5)).xy, _46.z))), 1.0);
    return out;
}


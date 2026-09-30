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

struct type_Scene
{
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    packed_float3 lightDir;
    float shininess;
    packed_float3 viewPos;
    float shadowSizeInv;
    packed_float3 warpCenter;
    float warpIntensity;
    packed_float3 bendDir;
    float ambientItensity;
    packed_float3 groundTint;
    float groundScale;
    packed_float3 lightColor;
    float warpScaleInv;
};

struct PSInstance_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PSInstance_in
{
    float3 in_var_WORLDPOS [[user(locn0)]];
    float3 in_var_NORMAL [[user(locn1)]];
    float4 in_var_COLOR [[user(locn3)]];
};

fragment PSInstance_out PSInstance(PSInstance_in in [[stage_in]], constant type_Scene& Scene [[buffer(1)]], texture2d<float> shadowMap [[texture(4)]], sampler shadowMapSampler [[sampler(5)]], float4 gl_FragCoord [[position]])
{
    PSInstance_out out = {};
    float3 _60 = -float3(Scene.lightDir);
    float3 _61 = fast::normalize(in.in_var_NORMAL);
    float2 _84 = step(fract(gl_FragCoord.xy * 0.5), float2(0.25));
    float4 _92 = Scene.vpShadowMatrix * float4(in.in_var_WORLDPOS, 1.0);
    float4 _95 = _92 / float4(_92.w);
    float2 _101 = ((_95.xy * float2(0.5, -0.5)) + float2(0.5)).xy;
    float _106 = _95.z;
    float3 _133 = (float3(Scene.lightColor) * ((in.in_var_COLOR.xyz * mix(0.20000000298023223876953125, 1.0, precise::max(0.0, dot(_61, _60)))) + float3(powr(precise::max(0.0, dot(_61, fast::normalize(fast::normalize(float3(Scene.viewPos) - in.in_var_WORLDPOS) + _60))), Scene.shininess)))) * mix(Scene.ambientItensity, 1.0, (((spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_101 + ((_84 + float2(-1.5, 0.5)) * Scene.shadowSizeInv)), _106) + spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_101 + ((_84 + float2(0.5)) * Scene.shadowSizeInv)), _106)) + spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_101 + ((_84 + float2(-1.5)) * Scene.shadowSizeInv)), _106)) + spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_101 + ((_84 + float2(0.5, -1.5)) * Scene.shadowSizeInv)), _106)) * 0.25);
    out.out_var_SV_Target = float4(_133, in.in_var_COLOR.w);
    return out;
}


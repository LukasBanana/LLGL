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

struct PSGround_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PSGround_in
{
    float2 in_var_TEXCOORD [[user(locn0)]];
    float3 in_var_WORLDPOS [[user(locn1)]];
};

fragment PSGround_out PSGround(PSGround_in in [[stage_in]], constant type_Scene& Scene [[buffer(1)]], texture2d<float> colorMap [[texture(2)]], texture2d<float> shadowMap [[texture(4)]], sampler colorMapSampler [[sampler(3)]], sampler shadowMapSampler [[sampler(5)]], float4 gl_FragCoord [[position]])
{
    PSGround_out out = {};
    float4 _69 = colorMap.sample(colorMapSampler, (in.in_var_TEXCOORD * Scene.groundScale)) * float4(Scene.groundTint[0], Scene.groundTint[1], Scene.groundTint[2], 1.0);
    float2 _75 = step(fract(gl_FragCoord.xy * 0.5), float2(0.25));
    float4 _83 = Scene.vpShadowMatrix * float4(in.in_var_WORLDPOS, 1.0);
    float4 _86 = _83 / float4(_83.w);
    float2 _92 = ((_86.xy * float2(0.5, -0.5)) + float2(0.5)).xy;
    float _97 = _86.z;
    out.out_var_SV_Target = float4((_69.xyz * float3(Scene.lightColor)) * mix(Scene.ambientItensity, 1.0, (((spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_92 + ((_75 + float2(-1.5, 0.5)) * Scene.shadowSizeInv)), _97) + spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_92 + ((_75 + float2(0.5)) * Scene.shadowSizeInv)), _97)) + spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_92 + ((_75 + float2(-1.5)) * Scene.shadowSizeInv)), _97)) + spvDepthCast(shadowMap).sample_compare(shadowMapSampler, (_92 + ((_75 + float2(0.5, -1.5)) * Scene.shadowSizeInv)), _97)) * 0.25), _69.w);
    return out;
}


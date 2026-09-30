#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

struct VSGround_out
{
    float2 out_var_TEXCOORD [[user(locn0)]];
    float3 out_var_WORLDPOS [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct VSGround_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VSGround_out VSGround(VSGround_in in [[stage_in]], constant type_Scene& Scene [[buffer(1)]])
{
    VSGround_out out = {};
    out.gl_Position = Scene.vpMatrix * float4(in.in_var_POSITION, 1.0);
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    out.out_var_WORLDPOS = in.in_var_POSITION;
    return out;
}


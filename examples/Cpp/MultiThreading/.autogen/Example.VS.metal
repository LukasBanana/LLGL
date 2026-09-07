#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Scene
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float3 lightVec;
};

struct VS_out
{
    float3 out_var_NORMAL [[user(locn0)]];
    float4 out_var_COLOR [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_Scene& Scene [[buffer(1)]])
{
    VS_out out = {};
    out.gl_Position = Scene.wvpMatrix * float4(in.in_var_POSITION, 1.0);
    out.out_var_NORMAL = fast::normalize((Scene.wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz);
    out.out_var_COLOR = float4(in.in_var_TEXCOORD, 1.0, 1.0);
    return out;
}


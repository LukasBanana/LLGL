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

struct VS_out
{
    float4 out_var_WORLDPOS [[user(locn0)]];
    float3 out_var_TEXCOORD [[user(locn1)]];
    float3 out_var_COLOR [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float2 in_var_TEXCOORD [[attribute(1)]];
    float3 in_var_COLOR [[attribute(2)]];
    float in_var_ARRAYLAYER [[attribute(3)]];
    float4 in_var_WMATRIX_0 [[attribute(4)]];
    float4 in_var_WMATRIX_1 [[attribute(5)]];
    float4 in_var_WMATRIX_2 [[attribute(6)]];
    float4 in_var_WMATRIX_3 [[attribute(7)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_Settings& Settings [[buffer(2)]])
{
    VS_out out = {};
    float4x4 in_var_WMATRIX = {};
    in_var_WMATRIX[0] = in.in_var_WMATRIX_0;
    in_var_WMATRIX[1] = in.in_var_WMATRIX_1;
    in_var_WMATRIX[2] = in.in_var_WMATRIX_2;
    in_var_WMATRIX[3] = in.in_var_WMATRIX_3;
    float2 _42 = Settings.animVec * in.in_var_POSITION.y;
    float4 _50 = in_var_WMATRIX * float4(in.in_var_POSITION.x + _42.x, in.in_var_POSITION.y, in.in_var_POSITION.z + _42.y, 1.0);
    out.gl_Position = Settings.vpMatrix * _50;
    out.out_var_WORLDPOS = _50;
    out.out_var_TEXCOORD = float3(in.in_var_TEXCOORD, in.in_var_ARRAYLAYER);
    out.out_var_COLOR = in.in_var_COLOR;
    return out;
}


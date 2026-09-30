#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    packed_float3 lightDir;
    float shininess;
    float4 viewPos;
    float4 albedo;
};

struct VS_out
{
    float4 out_var_WORLDPOS [[user(locn0)]];
    float4 out_var_NORMAL [[user(locn1)]];
    float2 out_var_TEXCOORD [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    VS_out out = {};
    float4 _39 = Settings.wMatrix * float4(in.in_var_POSITION, 1.0);
    out.gl_Position = Settings.vpMatrix * _39;
    out.out_var_WORLDPOS = _39;
    out.out_var_NORMAL = Settings.wMatrix * float4(in.in_var_NORMAL, 0.0);
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    return out;
}


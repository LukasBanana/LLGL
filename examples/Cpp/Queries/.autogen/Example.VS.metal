#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 color;
    float3 lightDir;
};

struct VS_out
{
    float3 out_var_NORMAL [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    VS_out out = {};
    out.gl_Position = Settings.wvpMatrix * float4(in.in_var_POSITION, 1.0);
    out.out_var_NORMAL = (Settings.wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz;
    return out;
}


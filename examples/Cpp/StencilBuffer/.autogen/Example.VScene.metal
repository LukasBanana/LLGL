#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4 lightDir;
    float4 diffuse;
};

struct VScene_out
{
    float4 out_var_NORMAL [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct VScene_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
};

vertex VScene_out VScene(VScene_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    VScene_out out = {};
    out.gl_Position = Settings.vpMatrix * (Settings.wMatrix * float4(in.in_var_POSITION, 1.0));
    out.out_var_NORMAL = Settings.wMatrix * float4(in.in_var_NORMAL, 0.0);
    return out;
}


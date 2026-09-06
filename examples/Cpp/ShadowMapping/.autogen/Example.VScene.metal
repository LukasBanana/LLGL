#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    float4 lightDir;
    float4 diffuse;
};

struct VScene_out
{
    float4 out_var_WORLDPOS [[user(locn0)]];
    float4 out_var_NORMAL [[user(locn1)]];
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
    float4 _33 = Settings.wMatrix * float4(in.in_var_POSITION, 1.0);
    out.gl_Position = Settings.vpMatrix * _33;
    out.out_var_WORLDPOS = _33;
    out.out_var_NORMAL = Settings.wMatrix * float4(in.in_var_NORMAL, 0.0);
    return out;
}


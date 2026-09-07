#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 wMatrixInv;
    float4x4 vpMatrix;
    float4x4 vpMatrixInv;
    packed_float3 lightDir;
    float shininess;
    packed_float3 viewPos;
    float threshold;
    packed_float3 albedo;
    float reflectance;
    int2 viewportExtent;
};

struct VScene_out
{
    float4 out_var_NDC [[user(locn0)]];
    float4 out_var_WORLDPOS [[user(locn1)]];
    float4 out_var_MODELPOS [[user(locn2)]];
    float4 out_var_NORMAL [[user(locn3)]];
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
    float4 _33 = float4(in.in_var_POSITION, 1.0);
    float4 _36 = Settings.wMatrix * _33;
    float4 _39 = Settings.vpMatrix * _36;
    out.gl_Position = _39;
    out.out_var_NDC = _39 / float4(_39.w);
    out.out_var_WORLDPOS = _36;
    out.out_var_MODELPOS = _33;
    out.out_var_NORMAL = Settings.wMatrix * float4(in.in_var_NORMAL, 0.0);
    return out;
}


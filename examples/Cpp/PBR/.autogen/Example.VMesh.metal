#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 cMatrix;
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float2 aspectRatio;
    float mipCount;
    float _pad0;
    float4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uint2 _pad1;
};

struct VMesh_out
{
    float3 out_var_TANGENT [[user(locn0)]];
    float3 out_var_BITANGENT [[user(locn1)]];
    float3 out_var_NORMAL [[user(locn2)]];
    float2 out_var_TEXCOORD [[user(locn3)]];
    float4 out_var_WORLDPOS [[user(locn4)]];
    float4 gl_Position [[position]];
};

struct VMesh_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
    float3 in_var_TANGENT [[attribute(2)]];
    float3 in_var_BITANGENT [[attribute(3)]];
    float2 in_var_TEXCOORD [[attribute(4)]];
};

vertex VMesh_out VMesh(VMesh_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    VMesh_out out = {};
    float4 _49 = Settings.wMatrix * float4(in.in_var_POSITION, 1.0);
    out.gl_Position = Settings.vpMatrix * _49;
    out.out_var_TANGENT = fast::normalize(Settings.wMatrix * float4(in.in_var_TANGENT, 0.0)).xyz;
    out.out_var_BITANGENT = fast::normalize(Settings.wMatrix * float4(in.in_var_BITANGENT, 0.0)).xyz;
    out.out_var_NORMAL = fast::normalize(Settings.wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz;
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    out.out_var_WORLDPOS = _49;
    return out;
}


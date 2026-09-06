#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Scene
{
    float4x4 vpMatrix;
};

struct type_PushConstant_ModelData
{
    packed_float3 lightVec;
    uint instance;
};

struct Transform
{
    float4x4 wMatrix;
};

struct type_StructuredBuffer_Transform
{
    Transform _m0[1];
};

struct VS_out
{
    float4 out_var_WORLDPOS [[user(locn0)]];
    float3 out_var_NORMAL [[user(locn1)]];
    float2 out_var_TEXCOORD [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_PushConstant_ModelData& model [[buffer(0)]], const device type_StructuredBuffer_Transform& transforms [[buffer(1)]], constant type_Scene& Scene [[buffer(3)]])
{
    VS_out out = {};
    float4 _54 = transforms._m0[model.instance].wMatrix * float4(in.in_var_POSITION, 1.0);
    out.gl_Position = Scene.vpMatrix * _54;
    out.out_var_WORLDPOS = _54;
    out.out_var_NORMAL = (transforms._m0[model.instance].wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz;
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    return out;
}


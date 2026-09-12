#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneView
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 lightVec;
    float4 baseColor;
};

struct type_PushConstant_DynamicState_t
{
    float texCoordScaleFront;
    float texCoordScaleBack;
    float interpolationFactor;
    float invertXAxis;
};

struct PMorphTargetMesh_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PMorphTargetMesh_in
{
    float3 in_var_NORMAL [[user(locn0)]];
    float2 in_var_TEXCOORD [[user(locn1)]];
    float in_var_FLIPFACE [[user(locn2), flat]];
};

fragment PMorphTargetMesh_out PMorphTargetMesh(PMorphTargetMesh_in in [[stage_in]], constant type_PushConstant_DynamicState_t& dynamicState [[buffer(0)]], constant type_SceneView& SceneView [[buffer(3)]], texture2d<float> paperDetailMap [[texture(4)]], texture2d<float> frontPageColorMap [[texture(6)]], texture2d<float> backPageColorMap [[texture(8)]], sampler paperDetailMapSampler [[sampler(5)]], sampler frontPageSampler [[sampler(7)]], sampler backPageSampler [[sampler(9)]], bool gl_FrontFacing [[front_facing]])
{
    PMorphTargetMesh_out out = {};
    float2 _74;
    if (gl_FrontFacing)
    {
        _74 = ((in.in_var_TEXCOORD - float2(0.5)) * dynamicState.texCoordScaleFront) + float2(0.5);
    }
    else
    {
        _74 = ((float2(1.0 - in.in_var_TEXCOORD.x, in.in_var_TEXCOORD.y) - float2(0.5)) * dynamicState.texCoordScaleBack) + float2(0.5);
    }
    float4 _86;
    if (gl_FrontFacing)
    {
        _86 = frontPageColorMap.sample(frontPageSampler, _74);
    }
    else
    {
        _86 = backPageColorMap.sample(backPageSampler, _74);
    }
    float4 _119 = (SceneView.baseColor + float4(paperDetailMap.sample(paperDetailMapSampler, _74).xyz - float3(0.5), 0.0)) * float4(mix(SceneView.baseColor.xyz, _86.xyz, float3(_86.w)), 1.0);
    out.out_var_SV_Target = float4(_119.xyz * mix(0.20000000298023223876953125, 1.0, dot(SceneView.lightVec.xyz, fast::normalize(in.in_var_NORMAL * (in.in_var_FLIPFACE * mix(-1.0, 1.0, float(gl_FrontFacing)))))), _119.w);
    return out;
}


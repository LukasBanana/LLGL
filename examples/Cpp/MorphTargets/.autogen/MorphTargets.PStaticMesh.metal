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

struct PStaticMesh_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PStaticMesh_in
{
    float3 in_var_NORMAL [[user(locn0)]];
    float2 in_var_TEXCOORD [[user(locn1)]];
};

fragment PStaticMesh_out PStaticMesh(PStaticMesh_in in [[stage_in]], constant type_SceneView& SceneView [[buffer(3)]], texture2d<float> paperDetailMap [[texture(4)]], texture2d<float> colorMap [[texture(6)]], sampler paperDetailMapSampler [[sampler(5)]], sampler colorMapSampler [[sampler(7)]])
{
    PStaticMesh_out out = {};
    float4 _43 = colorMap.sample(colorMapSampler, in.in_var_TEXCOORD);
    float4 _72 = (SceneView.baseColor + float4(paperDetailMap.sample(paperDetailMapSampler, in.in_var_TEXCOORD).xyz - float3(0.5), 0.0)) * float4(mix(SceneView.baseColor.xyz, _43.xyz, float3(_43.w)), 1.0);
    out.out_var_SV_Target = float4(_72.xyz * mix(0.20000000298023223876953125, 1.0, dot(SceneView.lightVec.xyz, fast::normalize(in.in_var_NORMAL))), _72.w);
    return out;
}


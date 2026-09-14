#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneView
{
    float4x4 cMatrix;
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float2 aspectRatio;
    float mipCount;
    float _pad0;
    float4 lightDir;
};

struct PMesh_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PMesh_in
{
    float3 in_var_TANGENT [[user(locn0)]];
    float3 in_var_BITANGENT [[user(locn1)]];
    float3 in_var_NORMAL [[user(locn2)]];
    float2 in_var_TEXCOORD [[user(locn3)]];
    float4 in_var_WORLDPOS [[user(locn4)]];
};

fragment PMesh_out PMesh(PMesh_in in [[stage_in]], constant type_SceneView& SceneView [[buffer(1)]], texturecube<float> skyBox [[texture(3)]], texture2d<float> colorMap [[texture(4)]], texture2d<float> normalMap [[texture(5)]], texture2d<float> roughnessMap [[texture(6)]], texture2d<float> metallicMap [[texture(7)]], sampler smpl [[sampler(2)]])
{
    PMesh_out out = {};
    float4 _66 = colorMap.sample(smpl, in.in_var_TEXCOORD);
    float4 _75 = roughnessMap.sample(smpl, in.in_var_TEXCOORD);
    float _76 = _75.x;
    float4 _79 = metallicMap.sample(smpl, in.in_var_TEXCOORD);
    float _80 = _79.x;
    float3 _85 = float3x3(fast::normalize(in.in_var_TANGENT), fast::normalize(in.in_var_BITANGENT), fast::normalize(in.in_var_NORMAL)) * ((normalMap.sample(smpl, in.in_var_TEXCOORD).xyz * 2.0) - float3(1.0));
    float3 _92 = fast::normalize((SceneView.cMatrix * float4(0.0, 0.0, 0.0, 1.0)).xyz - in.in_var_WORLDPOS.xyz);
    float3 _93 = _66.xyz;
    float3 _97 = abs(float3(0.02040817402303218841552734375));
    float3 _100 = mix(_97 * _97, _93, float3(_80));
    float3 _102 = fast::normalize(_92 + SceneView.lightDir.xyz);
    float _106 = fast::clamp(dot(_85, _92), 0.001000000047497451305389404296875, 1.0);
    float _108 = fast::clamp(dot(_85, _102), 0.001000000047497451305389404296875, 1.0);
    float _111 = _76 * _76;
    float _117 = _111 * 2.0;
    float _126 = _111 * _111;
    float _130 = ((_108 * _108) * (_126 - 1.0)) + 1.0;
    out.out_var_SV_Target = float4((_93 * (float3(fast::clamp(dot(_85, SceneView.lightDir.xyz), 0.0, 1.0)) + (skyBox.sample(smpl, (-fast::normalize(reflect(_92, _85))), level(_76 * SceneView.mipCount)).xyz * 0.20000000298023223876953125))) + (((((_100 + ((float3(1.0) - _100) * powr(1.0 - fast::clamp(dot(_92, _102), 0.0, 1.0), 5.0))) * ((2.0 * _106) / (_106 + sqrt(_117 + ((1.0 - _117) * (_106 * _106)))))) * (_126 / ((3.1415927410125732421875 * _130) * _130))) / float3(4.0 * _106)) * _80), _66.w);
    return out;
}


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

fragment PMesh_out PMesh(PMesh_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]], texturecube_array<float> skyBox [[texture(3)]], texture2d_array<float> colorMaps [[texture(4)]], texture2d_array<float> normalMaps [[texture(5)]], texture2d_array<float> roughnessMaps [[texture(6)]], texture2d_array<float> metallicMaps [[texture(7)]], sampler smpl [[sampler(2)]])
{
    PMesh_out out = {};
    float3 _73 = float3(in.in_var_TEXCOORD, float(Settings.materialLayer));
    float4 _77 = colorMaps.sample(smpl, _73.xy, uint(rint(_73.z)));
    float4 _86 = roughnessMaps.sample(smpl, _73.xy, uint(rint(_73.z)));
    float _87 = _86.x;
    float4 _90 = metallicMaps.sample(smpl, _73.xy, uint(rint(_73.z)));
    float _91 = _90.x;
    float3 _96 = float3x3(fast::normalize(in.in_var_TANGENT), fast::normalize(in.in_var_BITANGENT), fast::normalize(in.in_var_NORMAL)) * ((normalMaps.sample(smpl, _73.xy, uint(rint(_73.z))).xyz * 2.0) - float3(1.0));
    float3 _103 = fast::normalize((Settings.cMatrix * float4(0.0, 0.0, 0.0, 1.0)).xyz - in.in_var_WORLDPOS.xyz);
    float3 _104 = _77.xyz;
    float3 _108 = abs(float3(0.02040817402303218841552734375));
    float3 _111 = mix(_108 * _108, _104, float3(_91));
    float3 _113 = fast::normalize(_103 + Settings.lightDir.xyz);
    float _117 = fast::clamp(dot(_96, _103), 0.001000000047497451305389404296875, 1.0);
    float _119 = fast::clamp(dot(_96, _113), 0.001000000047497451305389404296875, 1.0);
    float _122 = _87 * _87;
    float _128 = _122 * 2.0;
    float _137 = _122 * _122;
    float _141 = ((_119 * _119) * (_137 - 1.0)) + 1.0;
    float4 _163 = float4(-fast::normalize(reflect(_103, _96)), float(Settings.skyboxLayer));
    out.out_var_SV_Target = float4((_104 * (float3(fast::clamp(dot(_96, Settings.lightDir.xyz), 0.0, 1.0)) + (skyBox.sample(smpl, _163.xyz, uint(rint(_163.w)), level(_87 * Settings.mipCount)).xyz * 0.20000000298023223876953125))) + (((((_111 + ((float3(1.0) - _111) * powr(1.0 - fast::clamp(dot(_103, _113), 0.0, 1.0), 5.0))) * ((2.0 * _117) / (_117 + sqrt(_128 + ((1.0 - _128) * (_117 * _117)))))) * (_137 / ((3.1415927410125732421875 * _141) * _141))) / float3(4.0 * _117)) * _91), _77.w);
    return out;
}


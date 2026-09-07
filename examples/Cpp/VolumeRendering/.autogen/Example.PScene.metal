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

struct PScene_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PScene_in
{
    float4 in_var_NDC [[user(locn0)]];
    float4 in_var_WORLDPOS [[user(locn1)]];
    float4 in_var_NORMAL [[user(locn3)]];
};

fragment PScene_out PScene(PScene_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]], texture3d<float> noiseTexture [[texture(2)]], texture2d<float> depthRangeTexture [[texture(3)]], sampler linearSampler [[sampler(4)]], float4 gl_FragCoord [[position]])
{
    PScene_out out = {};
    float3 _69 = fast::normalize(in.in_var_NORMAL.xyz);
    float3 _72 = -float3(Settings.lightDir);
    float3 _77 = fast::normalize(float3(Settings.viewPos) - in.in_var_WORLDPOS.xyz);
    float3 _91 = in.in_var_WORLDPOS.xyz * 0.0500000007450580596923828125;
    float4 _102 = noiseTexture.sample(linearSampler, (_91 * 12.0));
    float3 _109 = fract(((_91 * 150.0) + float3(_102.x * 9.0)) + (_77 * 0.100000001490116119384765625));
    float3 _111 = _109 * (float3(1.0) - _109);
    float4 _137 = Settings.vpMatrixInv * float4(in.in_var_NDC.xy, depthRangeTexture.read(uint2(int3(int2(gl_FragCoord.xy), 0).xy), 0).x, 1.0);
    float3 _141 = (_137 / float4(_137.w)).xyz;
    float _143 = distance(in.in_var_WORLDPOS.xyz, _141) * 0.078125;
    float _148;
    _148 = 0.0;
    float _145 = 0.0;
    int _150 = 0;
    for (; _150 < 64; )
    {
        float3 _154 = float3(_145);
        _145 += 0.015625;
        _148 += (smoothstep(0.5 - Settings.threshold, 0.5 + Settings.threshold, noiseTexture.sample(linearSampler, ((Settings.wMatrixInv * float4(mix(in.in_var_WORLDPOS.xyz, _141, _154), 1.0)).xyz * 0.5)).x) * _143);
        _150++;
        continue;
    }
    out.out_var_SV_Target = float4(((float3(Settings.albedo) * mix(0.20000000298023223876953125, 1.0, precise::max(0.0, dot(_69, _72)))) * mix(0.3499999940395355224609375, 1.5, 1.0 - exp(_148 * (-0.5)))) + float3((powr(precise::max(0.0, dot(_69, fast::normalize(_77 + _72))), Settings.shininess) * Settings.reflectance) + (fast::clamp(1.0 - (7.0 * ((_111.x + _111.y) + _111.z)), 0.0, 1.0) * powr(mix(0.20000000298023223876953125, 1.0, fast::clamp(dot(reflect(-_77, _69), _72), 0.0, 1.0)), 1.5))), 1.0);
    return out;
}


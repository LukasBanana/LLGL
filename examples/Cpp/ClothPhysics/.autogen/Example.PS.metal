#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneState
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 gravity;
    uint2 gridSize;
    uint2 _pad0;
    float damping;
    float dTime;
    float dStiffness;
    float _pad1;
    float4 lightVec;
};

struct PS_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct PS_in
{
    float4 in_var_NORMAL [[user(locn0)]];
    float2 in_var_TEXCOORD [[user(locn1)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_SceneState& SceneState [[buffer(0)]], texture2d<float> colorMap [[texture(4)]], sampler linearSampler [[sampler(5)]], bool gl_FrontFacing [[front_facing]])
{
    PS_out out = {};
    float4 _60 = colorMap.sample(linearSampler, in.in_var_TEXCOORD);
    out.out_var_SV_Target0 = float4(mix(_60.xyz, float3(in.in_var_TEXCOORD, 1.0), float3(0.5)).xyz * mix(0.20000000298023223876953125, 1.0, precise::max(0.0, dot(fast::normalize(in.in_var_NORMAL.xyz) * mix(1.0, -1.0, float(gl_FrontFacing)), -SceneState.lightVec.xyz))), _60.w);
    return out;
}


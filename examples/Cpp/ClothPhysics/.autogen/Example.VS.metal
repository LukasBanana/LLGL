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

struct VS_out
{
    float4 out_var_NORMAL [[user(locn0)]];
    float2 out_var_TEXCOORD [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float4 in_var_POS [[attribute(0)]];
    float4 in_var_NORMAL [[attribute(1)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_SceneState& SceneState [[buffer(0)]])
{
    VS_out out = {};
    out.gl_Position = SceneState.wvpMatrix * in.in_var_POS;
    out.out_var_NORMAL = SceneState.wMatrix * in.in_var_NORMAL;
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    return out;
}


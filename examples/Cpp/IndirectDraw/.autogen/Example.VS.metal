#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneState
{
    float time;
    uint numSceneObjects;
    float aspectRatio;
};

struct VS_out
{
    float4 out_var_COLOR [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    float2 in_var_COORD [[attribute(0)]];
    float4 in_var_COLOR [[attribute(1)]];
    float2 in_var_ROTATION_0 [[attribute(2)]];
    float2 in_var_ROTATION_1 [[attribute(3)]];
    float2 in_var_POSITION [[attribute(4)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_SceneState& SceneState [[buffer(2)]])
{
    VS_out out = {};
    float2x2 in_var_ROTATION = {};
    in_var_ROTATION[0] = in.in_var_ROTATION_0;
    in_var_ROTATION[1] = in.in_var_ROTATION_1;
    out.gl_Position = float4(((in_var_ROTATION * in.in_var_COORD) + in.in_var_POSITION) * float2(SceneState.aspectRatio, 1.0), 0.0, 1.0);
    out.out_var_COLOR = in.in_var_COLOR;
    return out;
}


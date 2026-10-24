#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneView
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    packed_float3 lightDir;
    float reflectionFading;
    float4 diffuse;
};

struct VSScene_out
{
    float3 out_var_NORMAL [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct VSScene_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
};

vertex VSScene_out VSScene(VSScene_in in [[stage_in]], constant type_SceneView& SceneView [[buffer(1)]])
{
    VSScene_out out = {};
    out.gl_Position = SceneView.vpMatrix * (SceneView.wMatrix * float4(in.in_var_POSITION, 1.0));
    out.out_var_NORMAL = (SceneView.wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz;
    return out;
}


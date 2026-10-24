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

struct VSStencil_out
{
    float4 gl_Position [[position]];
};

struct VSStencil_in
{
    float3 in_var_POSITION [[attribute(0)]];
};

vertex VSStencil_out VSStencil(VSStencil_in in [[stage_in]], constant type_SceneView& SceneView [[buffer(1)]])
{
    VSStencil_out out = {};
    out.gl_Position = SceneView.vpMatrix * (SceneView.wMatrix * float4(in.in_var_POSITION, 1.0));
    return out;
}


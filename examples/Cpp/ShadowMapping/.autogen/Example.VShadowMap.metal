#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    float4 lightDir;
    float4 diffuse;
};

struct VShadowMap_out
{
    float4 gl_Position [[position]];
};

struct VShadowMap_in
{
    float3 in_var_POSITION [[attribute(0)]];
};

vertex VShadowMap_out VShadowMap(VShadowMap_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    VShadowMap_out out = {};
    out.gl_Position = Settings.vpShadowMatrix * (Settings.wMatrix * float4(in.in_var_POSITION, 1.0));
    return out;
}


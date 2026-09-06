#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4 lightDir;
    float4 diffuse;
};

struct VStencil_out
{
    float4 gl_Position [[position]];
};

struct VStencil_in
{
    float3 in_var_POSITION [[attribute(0)]];
};

vertex VStencil_out VStencil(VStencil_in in [[stage_in]], constant type_Settings& Settings [[buffer(1)]])
{
    VStencil_out out = {};
    out.gl_Position = Settings.vpMatrix * (Settings.wMatrix * float4(in.in_var_POSITION, 1.0));
    return out;
}


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

struct VSky_out
{
    float4 out_var_VIEWRAY [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex VSky_out VSky(constant type_Settings& Settings [[buffer(1)]], uint gl_VertexIndex [[vertex_id]])
{
    VSky_out out = {};
    float4 _34 = float4((gl_VertexIndex == 2u) ? 3.0 : (-1.0), (gl_VertexIndex == 0u) ? 3.0 : (-1.0), 1.0, 1.0);
    out.gl_Position = _34;
    out.out_var_VIEWRAY = float4(_34.xy * Settings.aspectRatio, 1.0, 0.0);
    return out;
}


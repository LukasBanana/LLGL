#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_PushConstant_Scene_t
{
    float4x4 projection;
    float2 glyphAtlasInvSize;
};

struct VS_out
{
    float2 out_var_TEXCOORD [[user(locn0)]];
    float4 out_var_COLOR [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct VS_in
{
    int2 in_var_POSITION [[attribute(0)]];
    int2 in_var_TEXCOORD [[attribute(1)]];
    float4 in_var_COLOR [[attribute(2)]];
};

vertex VS_out VS(VS_in in [[stage_in]], constant type_PushConstant_Scene_t& scene [[buffer(0)]])
{
    VS_out out = {};
    out.gl_Position = scene.projection * float4(float(in.in_var_POSITION.x), float(in.in_var_POSITION.y), 0.0, 1.0);
    out.out_var_TEXCOORD = scene.glyphAtlasInvSize * float2(float(in.in_var_TEXCOORD.x), float(in.in_var_TEXCOORD.y));
    out.out_var_COLOR = in.in_var_COLOR;
    return out;
}


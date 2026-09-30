#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_PushConstant_ModelData
{
    packed_float3 lightVec;
    uint instance;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_NORMAL [[user(locn1)]];
    float2 in_var_TEXCOORD [[user(locn2)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_PushConstant_ModelData& model [[buffer(0)]], texture2d<float> colorMap [[texture(4)]], sampler colorMapSampler [[sampler(5)]])
{
    PS_out out = {};
    float4 _36 = colorMap.sample(colorMapSampler, in.in_var_TEXCOORD);
    out.out_var_SV_Target = float4(_36.xyz * precise::max(0.20000000298023223876953125, dot(fast::normalize(in.in_var_NORMAL), float3(model.lightVec))), _36.w);
    return out;
}


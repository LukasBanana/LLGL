#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneSettings
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 diffuse;
    float4 glossiness;
    packed_float3 lightDir;
    float intensity;
};

struct VScene_out
{
    float3 out_var_NORMAL [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct VScene_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
};

vertex VScene_out VScene(VScene_in in [[stage_in]], constant type_SceneSettings& SceneSettings [[buffer(1)]])
{
    VScene_out out = {};
    out.gl_Position = SceneSettings.wvpMatrix * float4(in.in_var_POSITION, 1.0);
    out.out_var_NORMAL = (SceneSettings.wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz;
    return out;
}


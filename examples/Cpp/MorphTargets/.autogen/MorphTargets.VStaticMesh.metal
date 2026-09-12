#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_SceneView
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 lightVec;
    float4 baseColor;
};

struct type_PushConstant_DynamicState_t
{
    float texCoordScaleFront;
    float texCoordScaleBack;
    float interpolationFactor;
    float invertXAxis;
};

struct VStaticMesh_out
{
    float3 out_var_NORMAL [[user(locn0)]];
    float2 out_var_TEXCOORD [[user(locn1)]];
    float out_var_FLIPFACE [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct VStaticMesh_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VStaticMesh_out VStaticMesh(VStaticMesh_in in [[stage_in]], constant type_PushConstant_DynamicState_t& dynamicState [[buffer(0)]], constant type_SceneView& SceneView [[buffer(3)]])
{
    VStaticMesh_out out = {};
    out.gl_Position = SceneView.wvpMatrix * float4(in.in_var_POSITION, 1.0);
    out.out_var_NORMAL = (SceneView.wMatrix * float4(in.in_var_NORMAL, 0.0)).xyz;
    out.out_var_TEXCOORD = ((in.in_var_TEXCOORD - float2(0.5)) * dynamicState.texCoordScaleFront) + float2(0.5);
    out.out_var_FLIPFACE = 1.0;
    return out;
}


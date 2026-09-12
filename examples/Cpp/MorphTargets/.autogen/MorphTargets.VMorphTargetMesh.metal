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

struct VMorphTargetMesh_out
{
    float3 out_var_NORMAL [[user(locn0)]];
    float2 out_var_TEXCOORD [[user(locn1)]];
    float out_var_FLIPFACE [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct VMorphTargetMesh_in
{
    float3 in_var_POSITIONA [[attribute(0)]];
    float3 in_var_NORMALA [[attribute(1)]];
    float3 in_var_POSITIONB [[attribute(2)]];
    float3 in_var_NORMALB [[attribute(3)]];
    float2 in_var_TEXCOORD [[attribute(4)]];
};

vertex VMorphTargetMesh_out VMorphTargetMesh(VMorphTargetMesh_in in [[stage_in]], constant type_PushConstant_DynamicState_t& dynamicState [[buffer(0)]], constant type_SceneView& SceneView [[buffer(3)]])
{
    VMorphTargetMesh_out out = {};
    float3 _48 = float3(dynamicState.interpolationFactor);
    float3 _49 = mix(in.in_var_POSITIONA, in.in_var_POSITIONB, _48);
    out.gl_Position = SceneView.wvpMatrix * float4(_49.x * dynamicState.invertXAxis, _49.yz, 1.0);
    out.out_var_NORMAL = (SceneView.wMatrix * float4(fast::normalize(mix(in.in_var_NORMALA, in.in_var_NORMALB, _48)), 0.0)).xyz;
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    out.out_var_FLIPFACE = dynamicState.invertXAxis;
    return out;
}


#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Scene
{
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    packed_float3 lightDir;
    float shininess;
    packed_float3 viewPos;
    float shadowSizeInv;
    packed_float3 warpCenter;
    float warpIntensity;
    packed_float3 bendDir;
    float ambientItensity;
    packed_float3 groundTint;
    float groundScale;
    packed_float3 lightColor;
    float warpScaleInv;
};

struct Instance
{
    float4 wMatrixRow0;
    float4 wMatrixRow1;
    float4 wMatrixRow2;
    float4 color;
};

struct type_StructuredBuffer_Instance
{
    Instance _m0[1];
};

struct type_PushConstant_Globals
{
    packed_float3 worldOffset;
    float bendIntensity;
    uint firstInstance;
};

struct VSInstance_out
{
    float3 out_var_WORLDPOS [[user(locn0)]];
    float3 out_var_NORMAL [[user(locn1)]];
    float2 out_var_TEXCOORD [[user(locn2)]];
    float4 out_var_COLOR [[user(locn3)]];
    float4 gl_Position [[position]];
};

struct VSInstance_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float3 in_var_NORMAL [[attribute(1)]];
    float2 in_var_TEXCOORD [[attribute(2)]];
};

vertex VSInstance_out VSInstance(VSInstance_in in [[stage_in]], constant type_PushConstant_Globals& globals [[buffer(0)]], constant type_Scene& Scene [[buffer(1)]], const device type_StructuredBuffer_Instance& instances [[buffer(2)]], uint gl_InstanceIndex [[instance_id]])
{
    VSInstance_out out = {};
    uint _62 = gl_InstanceIndex + globals.firstInstance;
    float4x3 _97 = float4x3(float3(instances._m0[_62].wMatrixRow0.xyz), float3(instances._m0[_62].wMatrixRow0.w, instances._m0[_62].wMatrixRow1.xy), float3(instances._m0[_62].wMatrixRow1.zw, instances._m0[_62].wMatrixRow2.x), float3(instances._m0[_62].wMatrixRow2.yzw));
    float3 _101 = (_97 * float4(in.in_var_POSITION + ((float3(Scene.bendDir) * in.in_var_POSITION.y) * globals.bendIntensity), 1.0)) + float3(globals.worldOffset);
    float3 _104 = _101 - float3(Scene.warpCenter);
    float _105 = length(_104);
    float3 _117 = _101 + ((fast::normalize(_104) * (1.0 / (1.0 + ((_105 * _105) * Scene.warpScaleInv)))) * Scene.warpIntensity);
    out.gl_Position = Scene.vpMatrix * float4(_117, 1.0);
    out.out_var_WORLDPOS = _117;
    out.out_var_NORMAL = _97 * float4(in.in_var_NORMAL, 0.0);
    out.out_var_TEXCOORD = in.in_var_TEXCOORD;
    out.out_var_COLOR = instances._m0[_62].color;
    return out;
}


#version 300 es

struct Instance
{
    vec4 wMatrixRow0;
    vec4 wMatrixRow1;
    vec4 wMatrixRow2;
    vec4 color;
};

layout(std140) uniform Scene
{
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpShadowMatrix;
    vec3 lightDir;
    float shininess;
    vec3 viewPos;
    float shadowSizeInv;
    vec3 warpCenter;
    float warpIntensity;
    vec3 bendDir;
    float ambientItensity;
    vec3 groundTint;
    float groundScale;
    vec3 lightColor;
    float warpScaleInv;
};

layout(std140) uniform instances
{
    Instance instancesElements[64];
};

struct type_PushConstant_Globals
{
    vec3 worldOffset;
    float bendIntensity;
    uint firstInstance;
};

uniform type_PushConstant_Globals globals;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
out vec3 v_WORLDPOS;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;
out vec4 v_COLOR;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    uint _63 = uint(gl_InstanceID) + globals.firstInstance;
    mat4x3 _98 = mat4x3(vec3(instancesElements[_63].wMatrixRow0.xyz), vec3(instancesElements[_63].wMatrixRow0.w, instancesElements[_63].wMatrixRow1.xy), vec3(instancesElements[_63].wMatrixRow1.zw, instancesElements[_63].wMatrixRow2.x), vec3(instancesElements[_63].wMatrixRow2.yzw));
    vec3 _102 = (_98 * vec4(POSITION + ((bendDir * POSITION.y) * globals.bendIntensity), 1.0)) + globals.worldOffset;
    vec3 _105 = _102 - warpCenter;
    float _106 = length(_105);
    vec3 _118 = _102 + ((normalize(_105) * (1.0 / (1.0 + ((_106 * _106) * warpScaleInv)))) * warpIntensity);
    gl_Position = vec4(_118, 1.0) * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _118;
    v_NORMAL = _98 * vec4(NORMAL, 0.0);
    v_TEXCOORD = TEXCOORD;
    v_COLOR = instancesElements[_63].color;
}


#version 450

struct Instance
{
    vec4 wMatrixRow0;
    vec4 wMatrixRow1;
    vec4 wMatrixRow2;
    vec4 color;
};

layout(binding = 1, std140) uniform Scene
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

layout(binding = 2, std430) readonly buffer type_StructuredBuffer_Instance
{
    Instance _m0[];
} instances;

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
layout(location = 0) out vec3 v_WORLDPOS;
layout(location = 1) out vec3 v_NORMAL;
layout(location = 2) out vec2 v_TEXCOORD;
layout(location = 3) out vec4 v_COLOR;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    uint _62 = uint(gl_InstanceID) + globals.firstInstance;
    mat4x3 _97 = mat4x3(vec3(instances._m0[_62].wMatrixRow0.xyz), vec3(instances._m0[_62].wMatrixRow0.w, instances._m0[_62].wMatrixRow1.xy), vec3(instances._m0[_62].wMatrixRow1.zw, instances._m0[_62].wMatrixRow2.x), vec3(instances._m0[_62].wMatrixRow2.yzw));
    vec3 _101 = (_97 * vec4(POSITION + ((bendDir * POSITION.y) * globals.bendIntensity), 1.0)) + globals.worldOffset;
    vec3 _104 = _101 - warpCenter;
    float _105 = length(_104);
    vec3 _117 = _101 + ((normalize(_104) * (1.0 / (1.0 + ((_105 * _105) * warpScaleInv)))) * warpIntensity);
    gl_Position = vec4(_117, 1.0) * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _117;
    v_NORMAL = _97 * vec4(NORMAL, 0.0);
    v_TEXCOORD = TEXCOORD;
    v_COLOR = instances._m0[_62].color;
}


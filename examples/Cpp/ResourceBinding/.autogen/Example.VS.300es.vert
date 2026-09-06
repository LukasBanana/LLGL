#version 300 es

struct Transform
{
    mat4 wMatrix;
};

layout(std140) uniform Scene
{
    layout(row_major) mat4 vpMatrix;
};

layout(std430) readonly buffer type_StructuredBuffer_Transform
{
    layout(row_major) Transform _m0[];
} transforms;

struct type_PushConstant_ModelData
{
    vec3 lightVec;
    uint instance;
};

uniform type_PushConstant_ModelData model;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
out vec4 v_WORLDPOS;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    vec4 _54 = vec4(POSITION, 1.0) * transforms._m0[model.instance].wMatrix;
    gl_Position = _54 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _54;
    v_NORMAL = (vec4(NORMAL, 0.0) * transforms._m0[model.instance].wMatrix).xyz;
    v_TEXCOORD = TEXCOORD;
}

